#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <signal.h>
#include "screen.h"
#define MAX_LEN 512
static pthread_t threadPID_Screen;//create our pthread
static pthread_mutex_t* pSendListMutex;//create our pthread mutex
static pthread_cond_t* pSendListCond;//create our pthread condition variable


void* sendScreenThread(void* dat){
    char messageSx[MAX_LEN]= "";//buffer we use for messages
    Data data = *(Data*)dat;//all our arguments are in the struct
    List* pList = data.pList;//List shared between receiverthread and screenthread
    pSendListMutex = data.mutex;//mutex to protect access to the list
    pSendListCond = data.cond;//condition variable for synchronization
    int error = 0;//used for error checking
    char* cancel = "!\n";//if message = cancel this means we must trigger shutdown
        while(1){
            //accessing critical section
            pthread_mutex_lock(pSendListMutex);
            {
                while(List_count(pList) < 1){
                    //if nothing in list to copy and remove then wait until signalled
                    pthread_cond_wait(pSendListCond , pSendListMutex);
                }
                strncpy(messageSx,(char*)List_last(pList),MAX_LEN);//take message from list
                error = write(1,(char*)List_last(pList),MAX_LEN);//write message to stdout
                free(List_remove(pList));//free message from list
            }
            pthread_mutex_unlock(pSendListMutex);

            //error checking
            if(error == -1){
                printf("failure TO WRTIE TO SCREEN");
                return NULL;
            }

            //checks if message = shutdown message
            //starts shutdown if true
           if(strcmp(messageSx, cancel) == 0 ){
                pthread_mutex_lock(pSendListMutex);
                {
                    pthread_cond_signal(data.terminate);
                    //wakes up main and signals to start shutdown process
                }
                pthread_mutex_unlock(pSendListMutex);
                sleep(100);//used to give time to threads to clear out before shutting them down
            }
        }

    // NOTE NEVER EXECUTES BECEAUSE THREAD IS CANCELLED
	return NULL;
}

void screen_Init(void* data)
{
   //create thread
    pthread_create(
        &threadPID_Screen,         // PID (by pointer)
        NULL,               // Attributes
        sendScreenThread,      // Function
        data);
}


void screen_Shutdown(void* dat)
{
    
    Data data = *(Data*)dat;
    
    // Cancel thread
    pthread_cancel(threadPID_Screen);
    // Waits for thread to finish
    pthread_join(threadPID_Screen, NULL);

    pthread_mutex_destroy(data.mutex);
    pthread_cond_destroy(data.cond);

    //destory mutex and condition variable

}
