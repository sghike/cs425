#ifndef __MP0_H_
#define __MP0_H_

#include <pthread.h>

/*** unicast interface ***/

/* Should be called by multicast_init */
void unicast_init(void);

/* Send a message to destination */
void usend(int destination, const char *message, int len);

/* Called *by* unicast interface when an incoming unicast message
 * arrives from source */
void receive(int source, const char *message, int len);

/*** partial multicast interface ***/

/* Call at start of program. */
void multicast_init(void);


/*** group maintenance ***/

extern int *mcast_members;
extern int mcast_num_members;
extern int my_id;
extern pthread_mutex_t member_lock;

/*** internals ***/

/* File where the list of current group members is stored */
#define GROUP_FILE  "GROUPLIST"

/* Minimum and maximum delay values, in microseconds */
#define MINDELAY    500000      /* 500ms */
#define MAXDELAY    10000000    /* 10s */

#define P_DROP      0.2         /* probability that a packet is dropped */

#endif
