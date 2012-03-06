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
int num_queue;
int my_seq;
int *my_vector;

typedef struct {
  char *str;
  int source;
  int *vector;
  int vector_len;
  int src_seq;
} mcast_msg;

typedef struct _queue_entry_ queue_entry;
struct _queue_entry_ {
  mcast_msg msg;
  queue_entry *next;
  queue_entry *prev;
};
queue_entry *queue_tail;
queue_entry *queue_head;
    
void my_vector_print();
int find_src_index(int source);
void parse_msg(const char *message, int *msg_len, int **msg_vector, 
               int *src_seq, int *msg_vector_len);
char * strip_msg(const char *message, int msg_len);
void send_neg_ack(const char *message, int msg_len, int source, int src_index,
                  int src_seq);
void respond_to_neg_ack(int source, const char *message);
void put_in_buffer_queue(const char *msg_str, int msg_len, int source, 
                         int *msg_vector, int msg_vector_len, int src_seq);
void remove_and_deliver_from_buffer_queue(queue_entry *queue_ptr);
int ready_for_delivery(int *msg_vector, int msg_vector_len, int msg_src_seq, 
                       int src_index);
int * check_missing(queue_entry *ptr, int source, int last_delivered,
                    int missing_upto, int *num_missing);


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
  debugprintf("New member %d in pings at process %d\n", member, my_id);
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
    mcast_ping_num_members = 0;
    mcast_ping_mem_alloc = 16;
    mcast_pings = (mcast_ping *)malloc(sizeof(mcast_ping) * 
                                       mcast_ping_mem_alloc);
    my_vector = (int *)calloc(mcast_ping_mem_alloc, sizeof(int));
    alive = (int *)calloc(mcast_ping_mem_alloc, sizeof(int));
    num_queue = 0;
    queue_tail = NULL;
    queue_head =NULL;
    my_seq = 0;
    
    unicast_init();
    
    // Create a thread to send heart beats to other processes.
    send_duration = 10;
    if (pthread_create(&send_beats_thread, NULL, &send_heart_beats, 
                       (void *)&send_duration) != 0) {
      fprintf(stderr, "Error in pthread_create inside multicast_init\n");
      exit(1);
    }
    // Create a thread to determine failures.
    flush_duration = 20;
    if (pthread_create(&flush_beats_thread, NULL, &flush_received_beats, 
                       (void *)&flush_duration) != 0) {
      fprintf(stderr, "Error in pthread_create inside multicast_init\n");
      exit(1);
    }
}

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
    // Record a hear beat received.
    if (strcmp(str, "NEG") == 0) {
      respond_to_neg_ack(source, message);
    } else if (strcmp(message, "HeartBeat from ") == 0) {
      for (i = 0; i < mcast_ping_num_members; i++) {
        if (mcast_members[i] == source) {
          mcast_pings[i] = 1;
          break;
        }
      }
    } else {
      int msg_len;
      int msg_src_seq = -1;
      int msg_vector_len = -1;
      int *msg_vector;
      parse_msg(message, &msg_len, &msg_vector, &msg_src_seq, &msg_vector_len);
      char *msg_str = strip_msg(message, msg_len);

      int src_index = find_src_index(source);
      
      // Casual ordering with vector timestamp.
      int deliver_buffer_discard = ready_for_delivery(msg_vector, 
                                                      msg_vector_len, 
                                                      msg_src_seq, src_index);
      if (deliver_buffer_discard == 0) {
        deliver(source, message);
//        deliver(source, msg_str);
        my_vector[src_index]++;
        
        if (num_queue != 0)
          remove_and_deliver_from_buffer_queue(queue_head);
      } else if (deliver_buffer_discard == 1) {
          put_in_buffer_queue(msg_str, msg_len, source, msg_vector, 
                              msg_vector_len, msg_src_seq);
      } else {
        /* This message was already delivered. Discard it.*/
      } 
      send_neg_ack(message, msg_len, source, src_index, msg_src_seq);
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
    my_seq++;
    sprintf(str, "%d;", my_seq);
    // Add my_seq to message.
    strcat(buf, str);
    t -= strlen(str);

    pthread_mutex_lock(&member_lock);
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

    debugprintf("Msg = %s\n", buf);
    // Multicast to everyone including me.
    for (i = 0; i < mcast_ping_num_members; i++) {
      if(alive[i]) usend(mcast_members[i], buf, strlen(buf) + 1);
    }
    pthread_mutex_unlock(&member_lock);
    
    free(str);
    free(buf);
}

/* Supporting functions from here on. */

int find_src_index(int source) {
  int i;
  for (i = 0; i < mcast_ping_num_members; i++) {
    if (mcast_members[i] == source) {
      return i;
    }
  }
  return -1;
}

void parse_msg(const char *message, int *msg_len, int **msg_vector, 
               int *src_seq, int *msg_vector_len) {
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
  *src_seq = atoi(str);
  
  for (j = 0, i = i + 1; i < strlen(message); i++, j++) {
    if (message[i] == ';') break;
    str[j] = message[i];
  }
  str[j] = '\0';
  *msg_vector_len = atoi(str);

  *msg_vector = (int *)calloc((*msg_vector_len), sizeof(int));
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

  free(str);
}

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

void neg_ack(int node_index, int msg_vector_val, int source, int src_index,
             int src_seq) {
  int i;
  int ask, reqst, reqd;
  int *missing;
  int num_missing;
  printf("msg_vector_val = %d\n", msg_vector_val);
  
  if (node_index == src_index) {
    reqst = (src_seq > (my_vector[node_index] + 1));
    missing = check_missing(queue_head, source, my_vector[node_index], 
                            src_seq - 1, &num_missing);
  } else {
    reqst = (msg_vector_val > my_vector[node_index]);
    missing = check_missing(queue_head, source, my_vector[node_index],
                            msg_vector_val, &num_missing);
  }

  if (reqst) {
    if (alive[node_index]) { // Ask original generator if alive.
      ask = node_index;
    } else if (alive[src_index]) { // Ask the source of message if alive.
      ask = src_index;
    } else { // No visible source of required messages.
      return;
    }
    if (num_missing > 0) {
      char *buf = (char *)malloc(sizeof(char) * 20);
      // Send negative ack for each missing message.
      for (i = 0; i < num_missing; i++) {
        sprintf(buf, "NEG;%d;", missing[i]);
        debugprintf("\nsending %d a negative ack for %d seq no.\n", ask,
                      missing[i]);
        usend(mcast_members[ask], buf, strlen(buf) + 1);
      }
      free(buf);
      return;
    }
  }
}

void send_neg_ack(const char *message, int msg_len, int source, int src_index,
                  int src_seq) {
  int i, j;
  int msg_vector_val;
  // Skip msg_len, msg_str, src_seq, msg_vector_len.
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
      neg_ack(curr, msg_vector_val, source, src_index, src_seq);
      j = 0; 
      curr++;
    } else {
      buf[j++] = message[i];
    }
  }
 
  free(buf);
}

void respond_to_neg_ack(int source, const char *message) {
  debugprintf("\n Responding to negative ack %s\n", message);
}

void put_in_buffer_queue(const char *msg_str, int msg_len, int source, 
                         int *msg_vector, int msg_vector_len, int src_seq) {
  debugprintf("Buffering in queue message from process %d of seq %d\n", source,
              src_seq);
  queue_entry *new_entry;
  int i;
  
  new_entry = (queue_entry *)malloc(sizeof(queue_entry));
  new_entry->msg.str = (char *)malloc((msg_len + 1) * sizeof(char));
  strcpy(new_entry->msg.str, msg_str);
  new_entry->msg.source = source;
  new_entry->msg.vector = (int *)malloc(msg_vector_len * sizeof(int));
  for (i = 0; i < msg_vector_len; i++)
    new_entry->msg.vector[i] = msg_vector[i];
  new_entry->msg.vector_len = msg_vector_len;
  new_entry->msg.src_seq = src_seq;

  if(num_queue == 0) {
    new_entry->next = NULL;
    new_entry->prev = NULL;
    queue_tail = new_entry;
    queue_head = queue_tail;
  } else {
    new_entry->prev = queue_tail;
    new_entry->next = NULL;
    queue_tail->next = new_entry;
    queue_tail = new_entry;
  }
  
  num_queue++;
}

/* Recursive function to deliver messages from the buffer queue. */
void remove_and_deliver_from_buffer_queue(queue_entry *queue_ptr) {
  // Base case.
  if (queue_ptr == NULL) return;

  int src_index = find_src_index(queue_ptr->msg.source);
  // Check if you can pop and deliver.
  int check = ready_for_delivery(queue_ptr->msg.vector,
                                 queue_ptr->msg.vector_len,
                                 queue_ptr->msg.src_seq, src_index);
  // Possible that a message could be ready for delivery, or it is a copy of an
  // already delivered message.
  if (check == 0) 
    deliver(queue_ptr->msg.source, queue_ptr->msg.str);
  if (check <= 0) { 
    queue_entry *ptr_to_free = queue_ptr;
    
    queue_entry *temp = queue_ptr->prev;
    queue_ptr = queue_ptr->next;
    if (temp != NULL) 
      temp->next = queue_ptr;
    else 
      queue_head = queue_ptr;
    if (queue_ptr != NULL) {
      queue_ptr->prev = temp;
      if (queue_ptr->next == NULL) 
        queue_tail = queue_ptr;
    }
    
    free(ptr_to_free->msg.str);
    free(ptr_to_free->msg.vector);
    free(ptr_to_free);
    num_queue--;
   
    // Delivering this message might have made another messafe available for
    // delivery, hence scan from the beginning of buffer queue.
    remove_and_deliver_from_buffer_queue(queue_head);
  } else {
    remove_and_deliver_from_buffer_queue(queue_ptr->next);
  }
}


/* Return 1 if message is to be buffered, 0 if delivered, -1 if discarded.*/
int ready_for_delivery(int *msg_vector, int msg_vector_len, int msg_src_seq, 
                       int src_index) {
  int i;
  int deliver_or_discard = 0;

  for (i = 0; i < msg_vector_len; i++) {
    if (i == src_index) {
      if (msg_src_seq > (my_vector[i] + 1)) 
        return 1; // Buffer.
      else if (msg_src_seq <= my_vector[i]) 
        deliver_or_discard = -1; // If we have seen this sequence no. from this
                                 // source, it means we have seen this message.
    } else {
      if (msg_vector[i] > my_vector[i]) 
        return 1; // Buffer.
    }
  }
  return deliver_or_discard;
}

void my_vector_print()
{
	int i;
	debugprintf("My current vector table\n");
	for(i = 0; i < 16; i++)
		debugprintf("[%d] ", my_vector[i]);
	debugprintf("\n");
}

/* Return the sequence numbers of messages from the given source, greater than 
   the last sequence number delivered from the source, and that are missing from
   the buffer queue.*/
int * check_missing(queue_entry *ptr, int source, int last_delivered, 
                    int missing_upto, int *num_missing) {
}

