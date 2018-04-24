#include "io_controller.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_system.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "include/priorities.h"
#include "include/pin_config.h"

#define _BUFFER_SIZE				(512)
#define _QUEUE_SIZE					(10)
#define _IO_CONTROLLER_MARKER_BYTE	(0x55)

typedef struct  __attribute__((packed))
{
	uint8_t marker;
	uint8_t address;
	uint16_t data;
} _io_controller_packet_t;

static const int _uart_num = UART_NUM_2;

static QueueHandle_t _rx_queue;
static QueueHandle_t _tx_queue;

static BaseType_t _rx_task;
static BaseType_t _tx_task;

static const uart_config_t _uart_config = {
    .baud_rate = 230400,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
};

void _RxTask(void* arg)
{
	uint16_t read_length = 0;
	_io_controller_packet_t rx_packet;
	while(1)
	{
		read_length = uart_read_bytes(_uart_num, (void*)&rx_packet, 3, portMAX_DELAY);
		if(read_length == 3)
		{
			xQueueSendToBack(_rx_queue, (void*)&rx_packet, (TickType_t) 0);
		}
	}
}

void _TxTask(void* arg)
{
	_io_controller_packet_t tx_packet;
	while(1)
	{
		if(xQueueReceive(_tx_queue, &tx_packet, portMAX_DELAY))
		{
			tx_packet.marker = _IO_CONTROLLER_MARKER_BYTE;
			uart_write_bytes(_uart_num, (const char*)&tx_packet, sizeof(_io_controller_packet_t));
		}
	}
}

void IoController_Create(void)
{

	//setup freeertos queues for tx and rx
	_rx_queue = xQueueCreate(_QUEUE_SIZE, sizeof(_io_controller_packet_t));
	_tx_queue = xQueueCreate(_QUEUE_SIZE, sizeof(_io_controller_packet_t));

	uart_param_config(_uart_num, &_uart_config);
	uart_set_pin(_uart_num, IO_CONTROLLER_UART_TXD_PIN, IO_CONTROLLER_UART_RXD_PIN, 0, 0);

	uart_driver_install(_uart_num, _BUFFER_SIZE * 2, 0, 0, NULL, 0);

	_rx_task = xTaskCreate(_RxTask, "uart_rx", 1024, NULL, IO_CONTROLLER_RX_THREAD_PRIORITY, NULL);
	_tx_task = xTaskCreate(_TxTask, "uart_tx", 1024, NULL, IO_CONTROLLER_TX_THREAD_PRIORITY, NULL);

}

uint32_t IoController_RxAvailable(void)
{
	return uxQueueMessagesWaiting(_rx_queue);
}

void IoController_Read(uint8_t address)
{
	_io_controller_packet_t packet;
	packet.address = address & ~(0x80);  //top bit must be cleared for read
	packet.data = 0;
	xQueueSendToBack(_tx_queue, &packet, 0);
}

void IoController_Write(uint8_t address, uint16_t data)
{
	_io_controller_packet_t packet;
	packet.address = address | 0x80; //top bit set for write
	packet.data = data;
	xQueueSendToBack(_tx_queue, &packet, 0);
}

bool IoController_GetRxData(uint8_t* address, uint16_t* data)
{
	_io_controller_packet_t packet;
	if(xQueueReceive(_rx_queue, &packet, 0))
	{
		*address = packet.address;
		*data = packet.data;
		return true;
	}
	else
	{
		return false;
	}
}