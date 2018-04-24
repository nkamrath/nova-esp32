#ifndef NVM_MANAGER_H_
#define NVM_MANAGER_H_

#include <stdint.h>

void NvmManager_Create(void);

uint64_t NvmManager_GetCachedAuthCounter(void);
void NvmManager_IncrementAuthCounter(void);
void NvmManager_WriteCache(void);
uint8_t NvmManager_GetPassword(char* password);

#endif