#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mp1.h"

pthread_t send_beats_thread;
pthread_t flush_beats_thread;
int mcast_ping_num_members; // Different variable needed since call to
                            // mcast_join is not lock protected in unicast.c.
int mcast_ping_mem_alloc;   // Same as above.
int flush_duration;
int send_duration;

typedef int mcast_ping;
mcast_ping *mcast_pings;

typedef struct {
  char *str;
  int seq;
} mcast_msg;
mcast_msg *mcast_msgs;
int my_seq;

void put_in_queue(int msg_src_seq);
void respond_to_neg_ack();

void *send_heart_beats(void *duration) {
    int i;
    char message[16];
    strcpy(message, "HeartBeat from ");
    message[16] = '\0';
    int send_duration = *((int *)duration);
  
    while (1) {
      pthread_mutex_lock(&member_lock);
      // send hearbeats to every other process
      for(i = 0; i < mcast_num_members; i++) {
        if (my_id != mcast_members[i]) {
          debugprintf("%d sending message to %d\n",my_id, mcast_members[i]);
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
      mcast_msgs = realloc(mcast_msgs, 
                           mcast_ping_mem_alloc * sizeof(mcast_msgs[0]));
      if (!mcast_pings || !mcast_msgs) {
          perror("realloc");
          exit(1);
      }
  }
  // Record a dummy heart beat. If the process does not send another heart 
  // beat in send_duration, it will declared as failed.
  mcast_pings[mcast_ping_num_members] = 1;
  // Initialize the message field to a default value. The algorithm assumes that
  // this default message has already been delivered by my_id.
  mcast_msgs[mcast_ping_num_members].str = NULL;
  mcast_msgs[mcast_ping_num_members].seq = 0;
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
        if (mcast_members[i] != my_id) {
          if (mcast_pings[i]) {
            mcast_pings[i] = 0;
          } else {
            fprintf(stderr, "Process %d has detected that process %d has crashed!\n", 
                    my_id, mcast_members[i]); 
            // Bring the last process at this location and reduce the total
            // number. Inspect the new process at i.
            mcast_members[i] = mcast_members[mcast_num_members - 1];
            mcast_num_members--;
            mcast_pings[i] = mcast_pings[mcast_ping_num_members - 1];
            mcast_msgs[i] = mcast_msgs[mcast_ping_num_members - 1];
            mcast_ping_num_members--;
            i--;
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
    mcast_msgs = (mcast_msg *)malloc(sizeof(mcast_msg) * mcast_ping_mem_alloc);
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

int find_recorded_seq(int source, int *index) {
  int i;
  for (i = 0; i < mcast_ping_num_members; i++) {
    if (mcast_members[i] == source) {
      *index = i;
      return mcast_msgs[i].seq;
    }
  }
  return -1;
}

int find_msg_seq(const char *message) {
  int i, j;
  char *str = (char *)malloc(sizeof(char) * 20);
  for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
    str[i] = message[i];
  }
  str[i] = '\0';

  int msg_len = atoi(str);
  for (j = 0, i = i + msg_len + 2; i < strlen(message); i++, j++) {
    if (message[i] == ';') break;
    str[j] = message[i];
  }
  str[j] = '\0';

  int val = atoi(str);
  free(str);
  return val;
}

void neg_ack(int node, int node_msg_seq, int source, int index) {
  int i, j;
  int ask;
  char *buf = (char *)malloc(sizeof(char) * 20);

  for (i = 0; i < mcast_ping_num_members; i++) {
    if (node == mcast_members[i]) {
      if (node_msg_seq > mcast_msgs[i].seq) {
        if (mcast_pings[i]) { // Ask original sender if alive.
          ask = i;
        } else if (mcast_pings[index]) { // Ask the sender of ack if alive.
          ask = source;
        } else { // No visible source of required messages.
          return;
        }
        int reqd = node_msg_seq - mcast_msgs[i].seq;
        printf("\n$$$$ in here reqd = %d\n", reqd);
        if (reqd) {
          sprintf(buf, "NEG;%d;%d", mcast_msgs[i].seq + 1, reqd);
          debugprintf("\nsending %d a negative ack\n",ask);
          usend(ask, buf, strlen(buf) + 1);
          return;
        }
      }
    }
  }
  free(buf);
}
    
        
void send_neg_ack(const char *message, int source, int src_index) {
  int i, j, k;
  int node, node_msg_seq;
  char *buf = (char *)malloc(sizeof(char) * 20);
  char *str = (char *)malloc(sizeof(char) * 20);
  for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
    str[i] = message[i];
  }
  str[i] = '\0';

  int msg_len = atoi(str);
  for (j = 0, i = i + msg_len + 2; i < strlen(message); i++, j++) {
    if (message[i] == ';') break;
  }
 
  for (j = 0, i = i + 1; i < strlen(message); i++) {
    if (message[i] == ';') {
      buf[j] = '\0';
      node_msg_seq = atoi(buf);
      neg_ack(node, node_msg_seq, source, src_index);
      j = 0; 
    } else if (message[i] == ',') {
      buf[j] = '\0';
      node = atoi(buf);
      j = 0;
    } else {
      buf[j++] = message[i];
    }
  }
 
  free(str);
  free(buf);
}

void receive(int source, const char *message, int len) {
    assert(message[len-1] == 0);
    int i;
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
      int msg_src_seq = find_msg_seq(message);
      assert(msg_src_seq != -1);
      int index = -1;
      int known_src_seq = find_recorded_seq(source, &index);
      assert((known_src_seq != -1) && (index != -1));
    
      if (msg_src_seq == (known_src_seq + 1)) {
        debugprintf("Delivering a message of seq %d\n",msg_src_seq);
        deliver(source, message);
        mcast_msgs[index].seq++;
      } else if (msg_src_seq > (known_src_seq + 1)) {
          put_in_queue(msg_src_seq);
      } /* else {
         This message was already delivered. Do nothing.
      } */
      send_neg_ack(message, source, index);
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

    pthread_mutex_lock(&member_lock);

    // Increment sequence number.
    my_seq++;
    sprintf(str, "%d;", my_seq);
    // Add my_seq to message.
    strcat(buf, str);
    t -= strlen(str);

    // Piggyback acknowledgements on message.
    for (i = 0; i < mcast_ping_num_members; i++) {
      sprintf(str, "%d,%d;", mcast_members[i], mcast_msgs[i].seq);
      t -= strlen(str);
      if (t < 0) {
        buf = realloc(buf, sizeof(char) * (strlen(buf) + 100));
        t += 100;
      }
      strcat(buf, str);
    }

    debugprintf("Msg = %s\n", buf);
    // Multicast.
    for (i = 0; i < mcast_num_members; i++) {
        usend(mcast_members[i], buf, strlen(buf)+1);
    }

    pthread_mutex_unlock(&member_lock);
    
    free(str);
    free(buf);
}

void respond_to_neg_ack(int source, const char *message) {
  debugprintf("\n Responding to negative ack %s\n", message);
}

void put_in_queue(int msg_src_seq) {
  debugprintf("Putting in queue message seq %d\n", msg_src_seq);
}
