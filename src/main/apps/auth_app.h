#ifndef _BLUETOOTH_AUTH_APP_H_
#define _BLUETOOTH_AUTH_APP_H_

#include <stdint.h>

void AuthApp_Create(void);
void AuthApp_IncrementCounter(void);
uint64_t AuthApp_GetCounter(void);

#endif