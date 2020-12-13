#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "screen.h"
#include "keyboard.h"
#include "sender.h"
#include "recv.h"

int main(int argc, char** args)
{   List* pSendList = List_create();//create the lists
    List* pRecvList = List_create();
    int sockfd = create_Socket(args);//create our socket and store file descriptor into variable
    pthread_mutex_t sendListMutex = PTHREAD_MUTEX_INITIALIZER;//create the mutexes and condition variables
    pthread_cond_t sendListCond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t recvListMutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t recvListCond = PTHREAD_COND_INITIALIZER;
    pthread_cond_t terminateThread = PTHREAD_COND_INITIALIZER;
    Data data = {args,&sockfd,pSendList,&sendListMutex,&sendListCond,&terminateThread};//initiliaze data structure
    Data data1 = {args,&sockfd,pRecvList,&recvListMutex,&recvListCond,&terminateThread};//contains the arguments we want to pass into threads

    receiver_init(&data1);//Initialize threads
    screen_Init(&data1);//Initialize threads
    
    sender_init(&data);//Initialize threads
    keyboard_Init(&data);//Initialize threads
    


    pthread_mutex_lock(data.mutex);
    {   
        pthread_cond_wait(data.terminate, data.mutex); //wait main thread until we get signal to start shutdown process
    }
    pthread_mutex_unlock(data.mutex);

    sleep(4);//let threads finish up their last operations
    receiver_Shutdown();//Shutting down the threads
    screen_Shutdown(&data1);//Shutting down the threads
    sender_Shutdown();//Shutting down the threads
    keyboard_Shutdown(&data);//Shutting down the threads
    List_free(data.pList,free);//free the lists
    List_free(data1.pList,free);//free the lists
    
    return 0;
}
/*
Read input to a reusable buffer

Allocate space for char* based on buffer size

Copy buffer to char*

add newly copied char* to list (note it doesn't have to be void*)
*/