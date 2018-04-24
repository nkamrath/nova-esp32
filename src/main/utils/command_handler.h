#ifndef COMMAND_HANDLER_H_
#define COMMAND_HANDLER_H_

#include <stdint.h>

typedef void(*bluetooth_command_callback_t)(void* data, uint8_t length);

void CommandHandler_Create(void);
void CommandHandler_RegisterCallback(uint8_t type, bluetooth_command_callback_t callback);

#endif