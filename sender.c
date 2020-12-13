#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>			// for strncmp()
#include <unistd.h>			// for close()
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "sender.h"
#define MAX_LEN 512
static pthread_t threadPID_Sender;//create our pthread
struct addrinfo *servinfo2;// will point to the results. used for sending
static pthread_mutex_t* pSendListMutex;//create our pthread mutex
static pthread_cond_t* pSendListCond;//create our pthread condition variable


int create_Socket(char** args){
    int sockfd;//socket file descriptor
    struct addrinfo hints, *servinfo, *p; // will point to the results
    int rv;//used for error checkings

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, args[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);//if unable to bind, close and give error
            perror("listener: bind");
            continue;
        }

        break;
    }
    //error check
    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);; // free the linked-list
    return sockfd;


}
void* SenderThread(void* dat){
    struct addrinfo hints, *p;//need to setup socket
    int rv;//used for error checking
    int numbytes;//used to see how many bytes are read
    Data data = *(Data*)dat;//all our arguments are in the struct
    char** args = data.args;//array with ports and hostnames/ip 
    int sockfd = *data.sockfd;//socket file descriptor
    List* pList = data.pList;//List shared between senderthread and keyboardthread
    pSendListMutex = data.mutex;//mutex to protect access to the list
    pSendListCond = data.cond;//condition variable for synchronization
    char* cancel = "!\n";//if message = cancel this means we must trigger shutdown
    
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;// don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;//udp socket
    
    //setup structures in order to setup networking
    if ((rv = getaddrinfo(args[2], args[3], &hints, &servinfo2)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return NULL;
    }
    p = servinfo2;
    

    while (1) {
        char messageSx[MAX_LEN] = "";//buffer we use for messages
        //accessing critical section
        pthread_mutex_lock(pSendListMutex);
        {   
            while(List_count(pList) < 1){
                //if nothing in list to copy and remove then wait until signalled
                pthread_cond_wait(pSendListCond , pSendListMutex);
            }
            strncpy(messageSx,(char*)List_last(pList),MAX_LEN);//take message from list
            //send message to other s-talk
            if ((numbytes = sendto(sockfd, (char*)List_last(pList), MAX_LEN, 0,p->ai_addr, p->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
            }
            free(List_remove(pList));//free message from list
        }
        pthread_mutex_unlock(pSendListMutex);

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
    //never hits this
        return NULL;
}

void sender_init(void* data)
{
    pthread_create(
        &threadPID_Sender,         // PID (by pointer)
        NULL,               // Attributes
        SenderThread,      // Function
        data);
}

void sender_Shutdown(void)
{   
    // Cancel thread
    pthread_cancel(threadPID_Sender);
     
    // Waits for thread to finish
    pthread_join(threadPID_Sender, NULL);

    
    freeaddrinfo(servinfo2); ; // free the linked-list
}



