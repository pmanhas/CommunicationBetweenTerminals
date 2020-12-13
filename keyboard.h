#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
#include "sender.h"

void* receiveKeyboardThread(void* unused);
void keyboard_Init(void* data);
void keyboard_Shutdown(void* data);
#endif