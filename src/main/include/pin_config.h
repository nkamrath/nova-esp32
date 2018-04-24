#ifndef _PIN_CONFIG_H_
#define _PIN_CONFIG_H_

#include "driver/gpio.h"

#define IO_CONTROLLER_UART_TXD_PIN		(GPIO_NUM_16)
#define IO_CONTROLLER_UART_RXD_PIN		(GPIO_NUM_17)


#define MAIN_POWER_SWITCH_PIN							(GPIO_NUM_14)
#define MAIN_POWER_SWITCH_PIN_CONFIG {					\
	.pin_bit_mask = (1<<MAIN_POWER_SWITCH_PIN),			\
	.mode = GPIO_MODE_OUTPUT,							\
	.pull_up_en = GPIO_PULLUP_DISABLE,					\
	.pull_down_en = GPIO_PULLDOWN_DISABLE,				\
	.intr_type = GPIO_INTR_DISABLE						\
}

#endif