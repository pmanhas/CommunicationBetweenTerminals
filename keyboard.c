#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <pthread.h>
#include <signal.h>
#include "keyboard.h"

static pthread_t threadPID_Keyboard; //create our pthread
static char* msg;//what we will use for dynamic allocation to add to the list
static pthread_mutex_t* pSendListMutex;// will point to the results. used for sending
static pthread_cond_t* pSendListCond;//create our pthread mutex
#define MAX_LEN 512


void* receiveKeyboardThread(void* dat){
    Data data = *(Data*)dat;//all our arguments are in the struct
    List* pList = data.pList;//List shared between senderthread and keyboardthread
    pSendListMutex = data.mutex;//mutex to protect access to the list
    pSendListCond = data.cond;//condition variable for synchronization
    char* cancel = "!\n";//if message = cancel this means we must trigger shutdown
    int error = 0;//used for error checking

        while(1){
            char messageSx[MAX_LEN]= "";//buffer we use for messages
            if(NULL == fgets(messageSx,MAX_LEN,stdin)){
                printf("error READING STRING FROM KEYBOARD");
                return NULL;
            }
            //allocating memory for msg
            msg = (char*)malloc(MAX_LEN * sizeof(char));
             
             //checking if allocation worked
            if(msg == NULL)                     
            {
                printf("Error! memory not allocated.");
                return NULL;
            }

            strncpy(msg,messageSx,MAX_LEN);
            //accessing critical section
            pthread_mutex_lock(pSendListMutex);
            {
                //add keyboard input to list
                error = List_prepend(pList, msg);
                //signallin for any threads waiting on this condition variable
                pthread_cond_signal(pSendListCond);
            }
            pthread_mutex_unlock(pSendListMutex);

            //error checkings
            if(error == -1){
                printf("error passing STRING to LIST");
                return NULL;
            }

            //checks if message = shutdown message
            //starts shutdown if true
            if(strcmp(messageSx, cancel) == 0){
                pthread_mutex_lock(pSendListMutex);
                {   
                    //wakes up main and signals to start shutdown process
                    pthread_cond_signal(data.terminate);
                }
                pthread_mutex_unlock(pSendListMutex);
                sleep(100);//used to give time to threads to clear out before shutting them down

            }

        }

    // NOTE NEVER EXECUTES BECEAUSE THREAD IS CANCELLED
	return NULL;
}

void keyboard_Init(void* data)
{
   
    pthread_create(
        &threadPID_Keyboard,         // PID (by pointer)
        NULL,               // Attributes
        receiveKeyboardThread,      // Function
        data);
}


void keyboard_Shutdown(void* dat)
{
    
    
    Data data = *(Data*)dat;
    
    // Cancel thread
     pthread_cancel(threadPID_Keyboard);
    

    // Waits for thread to finish
    
    pthread_join(threadPID_Keyboard, NULL);

    //close socket and destroying mutex and condition variable
    close(*data.sockfd);
    pthread_mutex_destroy(data.mutex);
    pthread_cond_destroy(data.cond);
    pthread_cond_destroy(data.terminate);
}
