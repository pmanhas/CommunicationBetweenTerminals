

#ifndef _SENDER_H_
#define _SENDER_H_
#include "list.h"
#include <pthread.h>
//structure used to pass in all needed arguments/parameters
typedef struct Data
{ 
   char** args;//array with ports and hostnames/ip 
   int* sockfd; //socket file descriptor
   List* pList;//List structure to be shard
   pthread_mutex_t* mutex;//mutex to protect critical section
   pthread_cond_t* cond;//condition variable to synchronize
   pthread_cond_t* terminate;//condition variable to synchronize shutdown

}Data;

// Start background send thread
void sender_init(void* data);
//creates socket and binds it
int create_Socket(char** args);

// Stop background send thread and cleanup
void sender_Shutdown(void);

#endif