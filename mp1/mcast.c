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
int num_queue;

typedef int mcast_ping;
mcast_ping *mcast_pings;

typedef struct {
  char *str;
  int seq;
  int vector[16];
} mcast_msg;
mcast_msg *mcast_msgs;

typedef struct queueentry queue_entry;

struct queueentry{
  mcast_msg msg;
  queue_entry* next;
  queue_entry* prev;
};
queue_entry *queue_tail;
queue_entry *queue_head;
    

int my_seq;
// my vector table
int my_vector_table[16];
int my_vector_num;

void my_vector_print();
void put_in_queue(int index);
void respond_to_neg_ack();
void pop_out_queue(int source, int index, queue_entry* queue_ptr);

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
  for(i = 0; i < 16; i++)
  {
  	//Assumes no transaction is happening AND nothing is in the queue of the process that is being copied
  	mcast_msgs[mcast_ping_num_members].vector[i] = mcast_msgs[mcast_ping_num_members-1].vector[i] ;
  }
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
	int i = 0;
    mcast_ping_num_members = 0;
    mcast_ping_mem_alloc = 16;
    mcast_pings = (mcast_ping *)malloc(sizeof(mcast_ping) * 
                                       mcast_ping_mem_alloc);
    mcast_msgs = (mcast_msg *)malloc(sizeof(mcast_msg) * mcast_ping_mem_alloc);
    num_queue = 0;
    queue_tail = NULL;
    queue_head =NULL;
    
    my_seq = 0;
    
    unicast_init();
    
    //init my vector table
    for (i = 0; i < 16; i++)
    {
    	my_vector_table[i] = 0;
    }
    
    //find my vector number in the vector table
    for (i = 0; i < mcast_ping_num_members; i++)
    {
    	if (mcast_members[i] == my_id) 
    		my_vector_num = i;
    }

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

char* find_msg_str(const char *message)
{
	int i, j;
	char *str = (char *)malloc(sizeof(char) * 100);
	for (i = 0; i < strlen(message); i++) {
    if (message[i] == ';') break;
    str[i] = message[i];
  	}
  	str[i] = '\0';
  	
  	int msg_len = atoi(str);
  	
  	//debugprintf("length is %s\n", str);
  	//debugprintf("msg = %d\n", msg_len);
  	for (j = 0, i = i + 1; i < strlen(message); i++, j++) {
    	if(message[i] == ';') 
    	{
    		debugprintf("i = %d\n", i);
    		break;
    	}
    	str[j] = message[i];
  	}
  	//debugprintf("j = %d\n", j);
  	str[j] = '\0';
  	//debugprintf("%s\n", message);
  	//debugprintf("%s\n", str);
  	
  	return str;
	
}

int find_msg_vector(const char *message, int number)
{
	int val;
	int i, j;
	int k = 0;
	int l = 0;
	char str[100];
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
  	
  	//debugprintf("number = %d\n", number);
  	//debugprintf("I is %d, vector is %c\n", i+1, message[i+1]);
  	for (i = i+1; i < strlen(message) ;i++)
  	{
  		if(message[i] == ';') 
  		{
  			l++;
  			//debugprintf("l = %d\n", l);
  			if(l > number)
  				break;
  		}
  		else
  		{
  			if(l == number)
  			{
  				str[k] = message[i];
  				k++;
  			}
  		}
  	}
  	str[k] = '\0';
  	//debugprintf("value before atoi %s\n", str);
  	val = atoi(str);
  	//debugprintf("value is %d\n", val);
	//free(str);
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
      int msg_src_seq = find_msg_seq(message);
      assert(msg_src_seq != -1);
      int index = -1;
      int known_src_seq = find_recorded_seq(source, &index);
      assert((known_src_seq != -1) && (index != -1));
      
      //copy vector table from message
      for(j = 0; j < 16; j++)
      {
      	mcast_msgs[index].vector[j] = find_msg_vector(message, j);
      }
      
      //copy the message
      mcast_msgs[index].str = find_msg_str(message);
      
      debugprintf("Receving inc message\n");
      my_vector_print();
      debugprintf("INC message vector\n");
      for(i = 0; i < 16; i++)
		debugprintf("[%d] ", mcast_msgs[index].vector[i]);
	  debugprintf("\n");
      
      // Casual ordering with vector timestamp
      // msg_src_seq == (known_src_seq + 1)
      if (check_vector_table(mcast_msgs[index].vector, index)!= 0) {
        debugprintf("Delivering a message of seq %d\n",msg_src_seq);
        deliver(source, mcast_msgs[index].str);
        free(mcast_msgs[index].str);
        mcast_msgs[index].seq++;
        my_vector_table[index]++;
        
        // testing conditions
        //mcast_msgs[index].vector[2] = 1;
        
        
        my_vector_print();
        if(num_queue != 0)
        	pop_out_queue(source, index, queue_head);
      } 
      // msg_src_seq > (known_src_seq + 1) &&
      else if (check_vector_table(mcast_msgs[index].vector, index) == 0) {
          put_in_queue(index);
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
    // Increment my vector table
    my_vector_table[my_vector_num]++;
    
    sprintf(str, "%d;", my_seq);
    // Add my_seq to message.
    strcat(buf, str);
    t -= strlen(str);
    
    // Add my vector table to message
    for(i = 0; i < 16; i++)
    {
    	sprintf(str, "%d;", my_vector_table[i]);
    	strcat(buf,str);
    	t -= strlen(str);
    }
    

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
    // Multicast except to me
    for (i = 0; i < mcast_num_members; i++) {
        if(mcast_members[i] != my_id)
        	usend(mcast_members[i], buf, strlen(buf)+1);
    }

    pthread_mutex_unlock(&member_lock);
    
    free(str);
    free(buf);
}

void respond_to_neg_ack(int source, const char *message) {
  debugprintf("\n Responding to negative ack %s\n", message);
}

void put_in_queue(int index) {
	debugprintf("Putting in queue message from process index %d\n", index);
	queue_entry* new_entry;
	int i;
	
	if(num_queue == 0)
	{
		
		queue_tail = (queue_entry *)malloc(sizeof(queue_entry));
    	
    	queue_tail->next = NULL;
    	queue_tail->prev = NULL;
    	
    	new_entry = queue_tail;
    	queue_head = queue_tail;
    }
    else
	{
		new_entry = (queue_entry *)malloc(sizeof(queue_entry));
  		queue_tail->next = new_entry;
		new_entry->prev = queue_tail;
		new_entry->next = NULL;
	}
  	
	new_entry->msg.str = mcast_msgs[index].str;
	new_entry->msg.seq = mcast_msgs[index].seq;
	
	for(i = 0; i < 16; i++)
	  	new_entry->msg.vector[i] = mcast_msgs[index].vector[i];
    
	queue_tail = new_entry;
	num_queue++;
}

void pop_out_queue(int source, int index, queue_entry* queue_ptr) {
	debugprintf("Popping out queue entry from process index %d\n", index);
	int check;
	queue_entry* temp;
	queue_entry* ptr_to_free;
	
	//check if there is any entry in the queue
	if(queue_ptr == NULL)
		return;
	//check if you can pop and deliver
	check = check_vector_table(queue_ptr->msg.vector);
	if(check == 0)
	{
		deliver(source, queue_ptr->msg.str);
		num_queue--;
		ptr_to_free = queue_ptr;
		temp = queue_ptr->prev;
		queue_ptr = queue_ptr->next;
		queue_ptr->prev = temp;
		if(temp == NULL)
			queue_head = queue_ptr;
		if(queue_ptr->next == NULL)
			queue_tail = queue_ptr;	
		if(queue_ptr != NULL)
			pop_out_queue(source, index, queue_head);
		free(ptr_to_free->msg.str);
		free(ptr_to_free);
	}
	else
	{
		queue_ptr = queue_ptr->next;
		if(queue_ptr != NULL)
			pop_out_queue(source, index, queue_ptr);
	}
	return;
}



int check_vector_table(int* vector, int index) {
	int i;
	int send = -1;
	
	for(i = 0; i < 16; i++)
	{
		if(i == index)
		{	
			if(vector[i] == my_vector_table[i] + 1  )
				send = 0;
			else
				send = -1;
		}
		else
		{
			if(my_vector_table[i] >= vector[i])
				send = 0;
			else 
				send = -1;					
		}
	}
	return send;
}

void my_vector_print()
{
	int i;
	debugprintf("My current vector table\n");
	for(i = 0; i < 16; i++)
		debugprintf("[%d] ", my_vector_table[i]);
	debugprintf("\n");
}





