#include "dio_control.h"
#include "utils/command_handler.h"
#include "services/io_controller.h"
#include "esp_log.h"

#define _IO_CONTROLLER_WRITE_COMMAND_TYPE 0x01

typedef struct __attribute__((packed))
{
	uint8_t address;
	uint16_t data;
} _io_controller_write_command_t;

static bool _initialized = false;

static void _HandleIoControllerWriteCommand(void* data, uint8_t length)
{
	ESP_LOGE("IO_CONTROL", "command callback\n");
	if(length == sizeof(_io_controller_write_command_t))
	{
		_io_controller_write_command_t* command = (_io_controller_write_command_t*) data;
		IoController_Write(command->address, command->data);
	}
	else
	{
		ESP_LOGE("IO_CONTROL", "bad io controller write command length\n");
	}
}

void DioControl_Create(void)
{
	if(!_initialized)
	{
		CommandHandler_Create();
		CommandHandler_RegisterCallback(_IO_CONTROLLER_WRITE_COMMAND_TYPE, _HandleIoControllerWriteCommand);
		_initialized = true;
	}
}