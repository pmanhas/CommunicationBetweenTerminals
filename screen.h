#ifndef _SCREEN_H_
#define _SCREEN_H_
#include "sender.h"
//intializes and creates screen thread
void screen_Init(void* data);
//shuts down screen thread
void screen_Shutdown(void* data);
#endif
