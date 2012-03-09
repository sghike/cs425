#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mp1.h"

// Failure Detection.
pthread_t send_beats_thread;
pthread_t flush_beats_thread;
int mcast_ping_num_members; // Different variable needed since call to
                            // mcast_join is not lock protected in unicast.c.
int mcast_ping_mem_alloc;   // Same as above.
int flush_duration;
int send_duration;

typedef int mcast_ping;
mcast_ping *mcast_pings;
int *alive;

// Causally ordered reliable multicast.
int buffer_num_queue;
int sent_num_queue;
int deliv_num_queue;
int *my_vector;
int my_vector_num;

typedef struct {
  char *str;
  int generator;
  int gen_seq;
  int *vector;
  int vector_len;
} mcast_msg;

typedef struct _queue_entry_ queue_entry;
struct _queue_entry_ {
  mcast_msg msg;
  queue_entry *next;
  queue_entry *prev;
};
queue_entry *buffer_queue_tail;
queue_entry *buffer_queue_head;
queue_entry *sent_queue_tail;
queue_entry *sent_queue_head;
queue_entry *deliv_queue_head;
queue_entry *deliv_queue_tail;
    
void my_vector_print();
int find_process_index(int source);
void parse_msg(const char *message, int source, int *msg_len, int **msg_vector, 
               int *msg_vector_len, int *generator, int *gen_seq);
char * strip_msg(const char *message, int msg_len);
void send_neg_ack(const char *message, int msg_len, int src_index);
void respond_to_neg_ack(int requestor, int generator, int gen_seq);
void push_to_queue(queue_entry **head, queue_entry **tail, int *num_queue,
                   const char *msg_str, int msg_len, int *msg_vector, 
                   int msg_vector_len, int generator, int gen_seq);
void remove_and_deliver_from_buffer_queue(queue_entry *queue_ptr);
int ready_for_delivery(int *msg_vector, int msg_vector_len, int msg_gen_seq, 
                       int src_index);
int * check_missing(queue_entry *ptr, int source, int last_delivered,
                    int missing_upto, int *num_missing);
mcast_msg * find_message(queue_entry *ptr, int generator, int gen_seq);
void store_sent_message(const char *message);
void store_deliv_message(const char *msg_str, int msg_len, int *msg_vector, 
                         int msg_vector_len, int generator, int gen_seq);

/* Send heart beats.*/
void *send_heart_beats(void *duration) {
    int i;
    char message[16];
    strcpy(message, "HeartBeat from ");
    message[16] = '\0';
    int send_duration = *((int *)duration);
  
    while (1) {
      pthread_mutex_lock(&member_lock);
      // send hearbeats to every other process
      for(i = 0; i < mcast_ping_num_members; i++) {
        if ((my_id != mcast_members[i]) && alive[i]) {
          debugprintf("%d sending heart beat to %d\n", my_id, mcast_members[i]);
          usend(mcast_members[i], message, strlen(message) + 1);
        }
      }
      pthread_mutex_unlock(&member_lock);
      sleep(send_duration);
    }
}

void mcast_join(int member) {
  int i;
  pthread_mutex_lock(&member_lock);
  /* Start recording heart beats and messages of new member.*/
  fprintf(stderr, "New member %d in pings at process %d\n", member, my_id);
  if (mcast_ping_num_members == mcast_ping_mem_alloc) { /* make sure there's
                                                           enough space */
      mcast_ping_mem_alloc *= 2;
      mcast_pings = realloc(mcast_pings, 
                            mcast_ping_mem_alloc * sizeof(mcast_pings[0]));
      my_vector = realloc(my_vector,
                          mcast_ping_mem_alloc * sizeof(my_vector[0]));
      alive = realloc(alive, mcast_ping_mem_alloc * sizeof(alive[0]));
      if (!mcast_pings || !my_vector) {
          perror("realloc");
          exit(1);
      }
  }
  // Record a dummy heart beat. If the process does not send another heart
  // beat in send_duration, it will declared as failed.
  mcast_pings[mcast_ping_num_members] = 1;
  alive[mcast_ping_num_members] = 1;
  my_vector[mcast_ping_num_members] = 0;
  mcast_ping_num_members++;
  pthread_mutex_unlock(&member_lock);
}

/* Receive heart beats and check for failures. */
void *flush_received_beats(void *duration) {
    int i;
    int receive_duration = *((int *)(duration));
    
    while (1) {
      sleep(receive_duration);
      pthread_mutex_lock(&member_lock);
      // If a process other than myself has not sent me a heart beat for
      // receive_duration, I declare it as failed.
      for (i = 0; i < mcast_ping_num_members; i++) {
        if ((mcast_members[i] != my_id) && alive[i]) {
          if (mcast_pings[i]) {
            mcast_pings[i] = 0;
          } else {
            fprintf(stderr, "Process %d has detected that process %d has crashed!\n", 
                    my_id, mcast_members[i]); 
            alive[i] = 0;
            mcast_num_members--;
          }
        }
      }
      pthread_mutex_unlock(&member_lock);
    }
}

void multicast_init(void) {
    int i = 0;
    mcast_ping_num_members = 0;
    mcast_ping_mem_alloc = 16;
    mcast_pings = (mcast_ping *)malloc(sizeof(mcast_ping) * 
                                       mcast_ping_mem_alloc);
    my_vector = (int *)calloc(mcast_ping_mem_alloc, sizeof(int));
    alive = (int *)calloc(mcast_ping_mem_alloc, sizeof(int));
    buffer_num_queue = 0;
    buffer_queue_tail = NULL;
    buffer_queue_head =NULL;
    sent_num_queue = 0;
    sent_queue_head = NULL;
    sent_queue_tail = NULL;
    deliv_num_queue = 0;
    deliv_queue_head = NULL;
    deliv_queue_tail = NULL;
    
    unicast_init();
    
    pthread_mutex_lock(&member_lock);
    for (i = 0; i < mcast_num_members; i++) {
      if (mcast_members[i] == my_id) 
        my_vector_num = i;
    }
    pthread_mutex_unlock(&member_lock);

    // Create a thread to send heart beats to other processes.
    send_duration = 10;
    if (pthread_create(&send_beats_thread, NULL, &send_heart_beats, 
                       (void *)&send_duration) != 0) {
      fprintf(stderr, "Error in pthread_create inside multicast_init\n");
      exit(1);
    }
    // Create a thread to determine failures.
    flush_duration = 30;
    if (pthread_create(&flush_beats_thread, NULL, &flush_received_beats, 
                       (void *)&flush_duration) != 0) {
      fprintf(stderr, "Error in pthread_create inside multicast_init\n");
      exit(1);
    }
}

/* Receive message from other process, could be heartbeat, negative ack or
 * regular multicast message.*/
void receive(int source, const char *message, int len) {
    assert(message[len-1] == 0);
    int i;
    int j;
    char str[4];
    for(i = 0; i < 3 && i < strlen(message); i++) {
      str[i] = message[i];
    }
    str[i] = '\0';

    pthread_mutex_lock(&member_lock);
    if (strcmp(str, "NEG") == 0) {
      char *buf = (char *)malloc(sizeof(char) * 20);
      int generator, reqst_seq;
      for (i = 4, j = 0; i < strlen(message); i++) {
        if (message[i] == ';') {
          buf[j] = '\0';
          generator = atoi(buf);
          break;
        } else {
          buf[j++] = message[i];
        }
      }
      for (i = i + 1, j = 0; i < strlen(message); i++) {
        if (message[i] == ';') {
          buf[j] = '\0';
          reqst_seq = atoi(buf);
        } else {
          buf[j++] = message[i];
        }
      }
      respond_to_neg_ack(source, generator, reqst_seq);
    } else if (strcmp(message, "HeartBeat from ") == 0) {
      // Record a hear beat received.
      for (i = 0; i < mcast_ping_num_members; i++) {
        if (mcast_members[i] == source) {
          mcast_pings[i] = 1;
          break;
        }
      }
    } else { // Multicast chat message.
      int msg_len;
      int msg_gen_seq = -1;
      int msg_vector_len = -1;
      int generator;
      int *msg_vector;
      parse_msg(message, source, &msg_len, &msg_vector, &msg_vector_len, 
                &generator, &msg_gen_seq);
      char *msg_str = strip_msg(message, msg_len);

      int gen_index = find_process_index(generator);
      
      // Causal ordering with vector timestamp.
      int deliver_buffer_discard = ready_for_delivery(msg_vector, 
                                                      msg_vector_len, 
                                                      msg_gen_seq, gen_index);
      if (deliver_buffer_discard == 0) {
        deliver(generator, msg_str);
        my_vector[gen_index]++;
        store_deliv_message(msg_str, msg_len, msg_vector, msg_vector_len, 
                            generator, msg_gen_seq);
        
        if (buffer_num_queue != 0)
          remove_and_deliver_from_buffer_queue(buffer_queue_head);
      } else if (deliver_buffer_discard == 1) {
          push_to_queue(&buffer_queue_head, &buffer_queue_tail,
                        &buffer_num_queue, msg_str, msg_len, msg_vector,
                        msg_vector_len, generator, msg_gen_seq);
      } else {
        /* This message was already delivered. Discard it.*/
      } 
      int src_index = find_process_index(source);
      send_neg_ack(message, msg_len, src_index);
    }
    pthread_mutex_unlock(&member_lock);
}

/* Basic multicast implementation */
void multicast(const char *message) {
    int i;
    int msg_len = strlen(message);
    char *str = (char *)malloc(sizeof(char) * 20);
    sprintf(str, "%d", msg_len);
    char *buf = (char *)malloc(sizeof(char) * (strlen(str) + msg_len + 103)); 
    // First declare the length of message and then start the actual message, so
    // that we can know where the message ends.
    sprintf(buf, "%s;%s;", str, message);
    int t = 100;

    // Increment sequence number.
    my_vector[my_vector_num]++;

    pthread_mutex_lock(&member_lock);
    // Add generator_index to message.
    sprintf(str, "%d;", my_vector_num);
    strcat(buf, str);
    t -= strlen(str);

    // Append the number of mcast members I have seen until now, included the
    // crashed ones.
    sprintf(str, "%d;", mcast_ping_num_members);
    strcat(buf, str);
    t -= strlen(str);

    // Add my vector table to message.
    for(i = 0; i < mcast_ping_num_members; i++) {
      sprintf(str, "%d;", my_vector[i]);
      strcat(buf,str);
      t -= strlen(str);
      if (t < 0) {
        buf = realloc(buf, sizeof(char) * (strlen(buf) + 100));
        t += 100;
      }
    }

    // Multicast to everyone excluding me.
    for (i = 0; i < mcast_ping_num_members; i++) {
      if(alive[i] && (mcast_members[i] != my_id)) 
        usend(mcast_members[i], buf, strlen(buf) + 1);
    }
    store_sent_message(message);
    // Deliver to myself.
    deliver(my_id, message); 
    
    pthread_mutex_unlock(&member_lock);
    
    free(str);
    free(buf);
}

/* Supporting functions from here on. */

int find_process_index(int source) {
  int i;
  for (i = 0; i < mcast_ping_num_members; i++) {
    if (mcast_members[i] == source) {
      return i;
    }
  }
  return -1;
}

/* Extract info from the message string.*/
void parse_msg(const char *message, int source, int *msg_len, int **msg_vector, 
               int *msg_vector_len, int *generator, int *gen_seq) {
  int i, j;
  char *str = (char *)malloc(sizeof(char) * 20);
  for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
    str[i] = message[i];
  }
  str[i] = '\0';
  *msg_len = atoi(str);

  for (j = 0, i = i + (*msg_len) + 2; i < strlen(message); i++, j++) {
    if (message[i] == ';') break;
    str[j] = message[i];
  }
  str[j] = '\0';
  int gen_index = atoi(str);
  *generator = mcast_members[gen_index];

  for (j = 0, i = i + 1; i < strlen(message); i++, j++) {
    if (message[i] == ';') break;
    str[j] = message[i];
  }
  str[j] = '\0';
  *msg_vector_len = atoi(str);

  *generator = mcast_members[gen_index];
  *msg_vector = (int *)calloc((*msg_vector_len), sizeof(int));
  if (*msg_vector == NULL) {
    perror("calloc");
    exit(1);
  }
  int curr = 0;
  for (j = 0, i = i + 1; i < strlen(message); i++) {
    if (message[i] == ';') {
      str[j] = '\0';
      (*msg_vector)[curr++] = atoi(str);
      j = 0; 
    } else {
      str[j++] = message[i];
    }
  }
  *gen_seq = (*msg_vector)[gen_index];

  free(str);
}

/* Extract multicasted string from the message.*/
char * strip_msg(const char *message, int msg_len) {
  int i, j;
  char *str = (char *)malloc((msg_len + 1) * sizeof(char));

  for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
  }

  for (j = 0, i = i + 1; j < msg_len; i++, j++) {
    str[j] = message[i];
  }
  str[j] = '\0';

  return str;
}

/* Send negative ack if required.*/
void neg_ack(int node_index, int msg_vector_val, int src_index) {
  int i;
  int ask, reqst, reqd;
  int *missing;
  int num_missing;
  
  reqst = (msg_vector_val > my_vector[node_index]);

  if (reqst) {
    if (alive[node_index]) { // Ask original generator if alive.
      ask = node_index;
    } else if (alive[src_index]) { // Ask the source of message if alive.
      ask = src_index;
    } else { // No visible source of required messages.
      return;
    }
    missing = check_missing(buffer_queue_head, mcast_members[node_index], 
                            my_vector[node_index], msg_vector_val, 
                            &num_missing);
    if (num_missing) {
      char *buf = (char *)malloc(sizeof(char) * 20);
      // Send negative ack for each missing message.
      for (i = 0; i < num_missing; i++) {
        sprintf(buf, "NEG;%d;%d;", mcast_members[node_index], missing[i]);
        debugprintf("Sending %d a negative ack for %d seq no.\n", ask,
                    missing[i]);
        usend(mcast_members[ask], buf, strlen(buf) + 1);
      }
      free(buf);
      return;
    }
  }
}

/* Send negative ack to all those processes whose messages are missing, but
 * some other process has delivered them.*/
void send_neg_ack(const char *message, int msg_len, int src_index) {
  int i, j;
  int msg_vector_val;
  // Skip msg_len, msg_str, gen_seq, msg_vector_len.
  for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
  }
  for (i = i + msg_len + 2; i < strlen(message); i++) {
    if (message[i] == ';') break;
  }
  for (i = i + 1; i < strlen(message); i++) {
    if (message[i] == ';') break;
  }

  // Read vector values from the message.
  int curr = 0;
  char *buf = (char *)malloc(sizeof(char) * 20);
  for (j = 0, i = i + 1; i < strlen(message); i++) {
    if (message[i] == ';') {
      buf[j] = '\0';
      msg_vector_val = atoi(buf);
      neg_ack(curr, msg_vector_val, src_index);
      j = 0; 
      curr++;
    } else {
      buf[j++] = message[i];
    }
  }
  free(buf);
}

/* Respond with the request message.*/
void respond_to_neg_ack(int requestor, int generator, int gen_seq) {
  int i, generator_index;
  mcast_msg *m;
  debugprintf("Responding to negative ack of %d for message generated by %d of seq %d\n", requestor, generator, gen_seq);
  if (generator == my_id) {
    m = find_message(sent_queue_head, generator, gen_seq);
    assert(m != NULL);
  } else {
    m = find_message(deliv_queue_head, generator, gen_seq);
    assert(m != NULL);
  }
  
  int msg_len = strlen(m->str);
  char *str = (char *)malloc(sizeof(char) * 20);
  sprintf(str, "%d", msg_len);
  char *buf = (char *)malloc(sizeof(char) * (strlen(str) + msg_len + 103)); 
  // First declare the length of message and then start the actual message, so
  // that we can know where the message ends.
  sprintf(buf, "%s;%s;", str, m->str);
  int t = 100;
  
  // Add generator_index to message.
  for (i = 0; i < mcast_ping_num_members; i++) {
    if (mcast_members[i] == generator) {
      generator_index = i;
      break;
    }
  }
  sprintf(str, "%d;", generator_index);
  strcat(buf, str);
  t -= strlen(str);

  // Append the message vector length.
  sprintf(str, "%d;", m->vector_len);
  strcat(buf, str);
  t -= strlen(str);
  
  // Add my vector table to message.
  for(i = 0; i < m->vector_len; i++) {
    sprintf(str, "%d;", m->vector[i]);
    strcat(buf, str);
    t -= strlen(str);
    if (t < 0) {
      buf = realloc(buf, sizeof(char) * (strlen(buf) + 100));
      assert(buf != NULL);
      t += 100;
    }
  }

  if(alive[find_process_index(requestor)]) {
    usend(requestor, buf, strlen(buf) + 1);
  }
}

/* Store the message in a given queue.*/
void push_to_queue(queue_entry **head, queue_entry **tail, int *num_queue,
                   const char *msg_str, int msg_len, int *msg_vector, 
                   int msg_vector_len, int generator, int gen_seq) {
  if (num_queue == &buffer_num_queue) {
    debugprintf("Buffering in queue message generated by process %d of seq %d\n",
                 generator, gen_seq);
  }

  queue_entry *new_entry;
  int i;
  
  new_entry = (queue_entry *)malloc(sizeof(queue_entry));
  new_entry->msg.str = (char *)malloc((msg_len + 1) * sizeof(char));
  strcpy(new_entry->msg.str, msg_str);
  new_entry->msg.generator = generator;
  new_entry->msg.gen_seq = gen_seq;
  new_entry->msg.vector = (int *)malloc(msg_vector_len * sizeof(int));
  for (i = 0; i < msg_vector_len; i++)
    new_entry->msg.vector[i] = msg_vector[i];
  new_entry->msg.vector_len = msg_vector_len;

  if(*num_queue == 0) {
    new_entry->next = NULL;
    new_entry->prev = NULL;
    *tail = new_entry;
    *head = new_entry;
  } else {
    new_entry->prev = *tail;
    new_entry->next = NULL;
    (*tail)->next = new_entry;
    *tail = new_entry;
  }
  
  (*num_queue)++;
}

/* Recursive function to deliver messages from the buffer queue. */
void remove_and_deliver_from_buffer_queue(queue_entry *queue_ptr) {
  // Base case.
  if (queue_ptr == NULL) return;
  int gen_index = find_process_index(queue_ptr->msg.generator);
  // Check if you can pop and deliver.
  int check = ready_for_delivery(queue_ptr->msg.vector,
                                 queue_ptr->msg.vector_len,
                                 queue_ptr->msg.gen_seq, gen_index);
  // Possible that a message could be ready for delivery, or it is a copy of an
  // already delivered message.
  if (check == 0) {
    deliver(queue_ptr->msg.generator, queue_ptr->msg.str);
    my_vector[gen_index]++;
    store_deliv_message(queue_ptr->msg.str, strlen(queue_ptr->msg.str), 
                        queue_ptr->msg.vector, queue_ptr->msg.vector_len, 
                        queue_ptr->msg.generator, queue_ptr->msg.gen_seq);
  }
  if (check <= 0) { 
    queue_entry *ptr_to_free = queue_ptr;
    
    queue_entry *temp = queue_ptr->prev;
    queue_ptr = queue_ptr->next;
    if (temp != NULL) 
      temp->next = queue_ptr;
    else 
      buffer_queue_head = queue_ptr;
    if (queue_ptr != NULL) {
      queue_ptr->prev = temp;
      if (queue_ptr->next == NULL) 
        buffer_queue_tail = queue_ptr;
    }
    
    free(ptr_to_free->msg.str);
    free(ptr_to_free->msg.vector);
    free(ptr_to_free);
    buffer_num_queue--;
   
    // Delivering this message might have made another messafe available for
    // delivery, hence scan from the beginning of buffer queue.
    remove_and_deliver_from_buffer_queue(buffer_queue_head);
  } else {
    remove_and_deliver_from_buffer_queue(queue_ptr->next);
  }
}


/* Return 1 if message is to be buffered, 0 if delivered, -1 if discarded.*/
int ready_for_delivery(int *msg_vector, int msg_vector_len, int msg_gen_seq, 
                       int gen_index) {
  int i;
  int deliver_or_discard = 0;

  for (i = 0; i < msg_vector_len; i++) {
    if (i == gen_index) {
      if (msg_gen_seq > (my_vector[i] + 1)) 
        return 1; // Buffer.
      else if (msg_gen_seq <= my_vector[i]) {
        deliver_or_discard = -1; // If we have seen this sequence no. from this
                                 // source, it means we have seen this message.
        debugprintf("Discarding message with generator_seq = %d because my vector value = %d\n", msg_gen_seq, my_vector[i]);
      }
    } else {
      if (msg_vector[i] > my_vector[i]) 
        return 1; // Buffer.
    }
  }
  return deliver_or_discard;
}

/* Return the sequence numbers of messages from the given source, greater than 
   the last sequence number delivered from the source, and that are missing from
   the buffer queue.*/
int * check_missing(queue_entry *ptr, int generator, int last_delivered, 
                    int missing_upto, int *num_missing) {
  int i, j;
  int mismatch = missing_upto - last_delivered;
  int count = mismatch;
  int *candidates = (int *)malloc(mismatch * sizeof(int));
  for (i = 0; i < mismatch; i++) {
    candidates[i] = last_delivered + 1 + i;
  }

  while (ptr) {
    // If the message has the same generator, sequence > last_delivered and upto
    // missing_upto, and it has not already been marked in candidates[] as 
    // present.
    if ((ptr->msg.generator == generator) && (ptr->msg.gen_seq <= missing_upto)
        && (ptr->msg.gen_seq > last_delivered) 
        && (candidates[ptr->msg.gen_seq - 1 - last_delivered] != -1)) {
      candidates[ptr->msg.gen_seq - 1 - last_delivered] = -1;
      count--;
    }
    ptr = ptr->next;
  }

  *num_missing = count;
  int *missing_seq = NULL;
  if (count) {
    missing_seq = (int *)malloc(count * sizeof(int));
    for (i = 0, j = 0; i < mismatch; i++) {
      if (candidates[i] != -1) {
        missing_seq[j++] = candidates[i];
      }
    }
  }

  free(candidates);
  return missing_seq;
}

/* Find and return a message from a given queue.*/
mcast_msg * find_message(queue_entry *ptr, int generator, int reqst_seq) {
  while (ptr) {
    if (ptr->msg.generator == generator && ptr->msg.gen_seq == reqst_seq) {
      return &ptr->msg;
    }
    ptr = ptr->next;
  }
  return NULL;
}

/* Store the sent message in sent_queue.*/
void store_sent_message(const char *message) {
    push_to_queue(&sent_queue_head, &sent_queue_tail, &sent_num_queue,
                  message, strlen(message), my_vector, mcast_ping_num_members, 
                  my_id, my_vector[my_vector_num]);
    return;
}

/*Store the delivered message in deliv_queue.*/
void store_deliv_message(const char *msg_str, int msg_len, int *msg_vector, 
                         int msg_vector_len, int generator, int gen_seq) {
    push_to_queue(&deliv_queue_head, &deliv_queue_tail, &deliv_num_queue, 
                  msg_str, msg_len, msg_vector, msg_vector_len, generator,
                  gen_seq);
    return;
}
