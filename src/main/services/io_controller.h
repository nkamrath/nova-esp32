#ifndef _IO_CONTROLLER_H_
#define _IO_CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

void IoController_Create(void);

void IoController_Read(uint8_t address);

void IoController_Write(uint8_t address, uint16_t data);

bool IoController_GetRxData(uint8_t* address, uint16_t* data);

#endif