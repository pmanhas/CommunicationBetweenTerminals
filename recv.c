#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "recv.h"

#define MAX_LEN 512
static pthread_t threadPID_Receiver;//create our pthread
static pthread_mutex_t* pSendListMutex;//create our pthread mutex
static pthread_cond_t* pSendListCond;//create our pthread condition variable
static char* msg;//what we will use for dynamic allocation to add to the list

void* receiverThread(void* dat){
    struct sockaddr_storage their_addr;
    socklen_t addr_len = sizeof their_addr;
    int numbytes;//used to see how many bytes are read and error checking
    int error = 0;//used for error checking
    Data data = *(Data*)dat;//all our arguments are in the struct
    int sockfd = *data.sockfd;//socket file descriptor
    List* pList = data.pList;//List shared between receiverthread and screenthread
    pSendListMutex = data.mutex;//mutex to protect access to the list
    pSendListCond = data.cond;//condition variable for synchronization
    char messageRx[MAX_LEN] = "";//buffer we use for messages
    char* cancel = "!\n";//if message = cancel this means we must trigger shutdown
    

    while (1) {
        //receive some data 
        if ((numbytes = recvfrom(sockfd, messageRx, MAX_LEN , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        
        msg = (char*)malloc(MAX_LEN * sizeof(char));
        //checking if allocation turned out all right
        if(msg == NULL)                     
        {
            printf("Error! memory not allocated.");
            return NULL;
        }

        strncpy(msg,messageRx,MAX_LEN);
        //accessing critical section
        pthread_mutex_lock(pSendListMutex);
            {
                // adding recieved message to list
                error = List_prepend(pList, msg);
                pthread_cond_signal(pSendListCond);
                //wake up any process waiting on condition variable
            }
        pthread_mutex_unlock(pSendListMutex);

        //do an error check
        if(error == -1){
            printf("error passing STRING to LIST");
            return NULL;
        }

        //checks if message = shutdown message
        //starts shutdown if true
        if(strcmp(messageRx, cancel) == 0){
            pthread_mutex_lock(pSendListMutex);
            {
                pthread_cond_signal(data.terminate);
                //wakes up main and signals to start shutdown process
            }
            pthread_mutex_unlock(pSendListMutex);
            sleep(100);//used to give time to threads to clear out before shutting them down
        }
        
    }
    //never hits this
        return NULL;
}



void receiver_init(void* data)
{   //create thread
    pthread_create(
        &threadPID_Receiver,         // PID (by pointer)
        NULL,               // Attributes
        receiverThread,      // Function
        data);
}

void receiver_Shutdown(void)
{

    // Cancel thread
    pthread_cancel(threadPID_Receiver);
    // Waits for thread to finish
    pthread_join(threadPID_Receiver, NULL);
    
}


