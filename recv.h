
#ifndef _RECV_H_
#define _RECV_H_
#include "sender.h"

// Start background receive thread
void receiver_init(void* data);



// Stop background receive thread and cleanup
void receiver_Shutdown(void);

#endif