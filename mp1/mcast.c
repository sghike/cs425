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
  /* Start recording heart beats of new member.*/
  debugprintf("New member %d in pings at process %d\n", member, my_id);
  if (mcast_ping_num_members == mcast_ping_mem_alloc) { /* make sure there's
                                                           enough space */
      mcast_ping_mem_alloc *= 2;
      mcast_pings = realloc(mcast_pings, 
                            mcast_ping_mem_alloc * sizeof(mcast_pings[0]));
      if (mcast_pings == NULL) {
          perror("realloc");
          exit(1);
      }
  }
  // Record a dummy heart beat. If the process does not send another heart 
  // beat in send_duration, it will declared as failed.
  mcast_pings[mcast_ping_num_members] = 1;
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

    pthread_mutex_lock(&member_lock);

    // Record a hear beat received.
    for (i = 0; i < mcast_ping_num_members; i++) {
      if (mcast_members[i] == source) {
        mcast_pings[i] = 1;
        break;
      }
    }

    pthread_mutex_unlock(&member_lock);
//    deliver(source, message);
}

/* Basic multicast implementation */
void multicast(const char *message) {
    int i;

    pthread_mutex_lock(&member_lock);
    for (i = 0; i < mcast_num_members; i++) {
        usend(mcast_members[i], message, strlen(message)+1);
    }
    pthread_mutex_unlock(&member_lock);
}

