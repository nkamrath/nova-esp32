#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

void PowerManager_Create(void);
void PowerManager_RegisterActiveCallback(bool (*function_pointer)(void));
void PowerManager_EnterDeepSleep(uint32_t duration_ms);

#endif