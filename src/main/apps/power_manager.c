
#include "power_manager.h"
#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_deep_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "include/priorities.h"
#include "include/pin_config.h"
#include "services/bluetooth.h"
#include "utils/command_handler.h"
#include "utils/nvm_manager.h"

#include "esp_log.h"


#define _MAX_ACTIVE_CALLBACKS						(20)
#define _TICKS_TILL_SLEEP_FROM_NEVER_ACTIVE			(500 / portTICK_PERIOD_MS)		//400 ms
#define _TICKS_TILL_SLEEP_FROM_ACTIVE 				(10000 / portTICK_PERIOD_MS)  	//5 minutes
#define _AUTO_SLEEP_DURATION_MS						(10000)	//5 seconds
#define _MAIN_POWER_ON_COMMAND_TYPE					(0x03)
#define _MAIN_POWER_OFF_COMMAND_TYPE				(0x04)


typedef struct __attribute__((packed))
{
	uint8_t power_on_delay_seconds;
} _main_power_on_command_t;

typedef struct __attribute__((packed))
{
	uint8_t power_off_delay_seconds;
} _main_power_off_command_t;

static bool _initialized = false;
static uint32_t _last_activity_time = 0;
static bool _ever_active = false;
static BaseType_t _auto_sleep_thread;
static bool (*_active_callbacks[_MAX_ACTIVE_CALLBACKS])(void);
static bool _main_power_on = false;
static bool _next_main_power_state = false;
static TimerHandle_t _change_power_timer = NULL;


static void _StartChangePowerTimer(uint32_t time_seconds)
{
	if(xTimerIsTimerActive(_change_power_timer))
	{
		xTimerStop(_change_power_timer, 0);
	}

	uint32_t power_on_delay_ticks = ((time_seconds * 1000) / portTICK_PERIOD_MS);
	xTimerChangePeriod(_change_power_timer, power_on_delay_ticks, 0);
	xTimerStart(_change_power_timer, 0);
}

static void _HandleMainPowerOn(void* data, uint8_t length)
{
	if(length == sizeof(_main_power_on_command_t))
	{
		_main_power_on_command_t* command = (_main_power_on_command_t*)data;
		_next_main_power_state = true;
		
		_StartChangePowerTimer(command->power_on_delay_seconds);
	}
}

static void _HandleMainPowerOff(void* data, uint8_t length)
{
	if(length == sizeof(_main_power_off_command_t))
	{
		_main_power_off_command_t* command = (_main_power_off_command_t*)data;
		_next_main_power_state = false;

		_StartChangePowerTimer(command->power_off_delay_seconds);
	}
}

static void _TurnOnMainPower(void)
{
	_main_power_on = true;
	gpio_set_level(MAIN_POWER_SWITCH_PIN, 0);
}

static void _TurnOffMainPower(void)
{
	_main_power_on = false;
	gpio_set_level(MAIN_POWER_SWITCH_PIN, 1);
}

static void _ChangeMainPower(void* arg)
{
	if(_main_power_on != _next_main_power_state)
	{
		if(_next_main_power_state)
		{
			_TurnOnMainPower();
		}
		else
		{
			_TurnOffMainPower();
		}
	}
}

static void _AutoSleepThread(void* arg)
{
	uint32_t sleep_at_time = 0;
	uint32_t current_time = 0;
	while(1)
	{
		current_time = xTaskGetTickCount();

		for(int i = 0; i < _MAX_ACTIVE_CALLBACKS; i++)
		{
			if(_active_callbacks[i] && _active_callbacks[i]())
			{
				_ever_active = true;
				_last_activity_time = current_time;
				break;
			}
		}

		if(_ever_active)
		{
			sleep_at_time = _last_activity_time + _TICKS_TILL_SLEEP_FROM_ACTIVE;
		}
		else
		{
			sleep_at_time = _last_activity_time + _TICKS_TILL_SLEEP_FROM_NEVER_ACTIVE;
		}

		if(xTaskGetTickCount() >= sleep_at_time)
		{
			PowerManager_EnterDeepSleep(_AUTO_SLEEP_DURATION_MS);
		}
		vTaskDelay(_TICKS_TILL_SLEEP_FROM_NEVER_ACTIVE/2);
	}
}


void PowerManager_Create(void)
{
	if(!_initialized)
	{
		//create main power pin
		//we set the level before calling config to ensure it starts output high
		//so that the relay does not toggle on pin configuration
		gpio_set_level(MAIN_POWER_SWITCH_PIN, 1);
		gpio_config_t main_power_pin_config = MAIN_POWER_SWITCH_PIN_CONFIG;
		gpio_config(&main_power_pin_config);

		for(int i = 0; i < _MAX_ACTIVE_CALLBACKS; i++)
		{
			_active_callbacks[i] = NULL;
		}

		CommandHandler_Create();
		CommandHandler_RegisterCallback(_MAIN_POWER_ON_COMMAND_TYPE, _HandleMainPowerOn);
		CommandHandler_RegisterCallback(_MAIN_POWER_OFF_COMMAND_TYPE, _HandleMainPowerOff);

		_ever_active = false;
		_last_activity_time = xTaskGetTickCount();

		_auto_sleep_thread = xTaskCreate(_AutoSleepThread, "sleep_thrd", 1024, NULL, AUTO_SLEEP_THREAD_PRIORITY, NULL);

		_change_power_timer = xTimerCreate("main_pwr", 500, 0, 0, _ChangeMainPower);

		_initialized = true;
	}
}

void PowerManager_RegisterActiveCallback(bool (*function_pointer)(void))
{
	if(function_pointer)
	{
		for(int i = 0; i < _MAX_ACTIVE_CALLBACKS; i++)
		{
			if(_active_callbacks[i] == NULL)
			{
				_active_callbacks[i] = function_pointer;
				return;
			}
		}
	}
}

void PowerManager_EnterDeepSleep(uint32_t duration_ms)
{
	//if we ever had any activity, write nvm manager cache back to flash
	if(_ever_active)
	{
		NvmManager_WriteCache();
	}

   	esp_deep_sleep_enable_timer_wakeup(duration_ms*1000); //deep sleep timer (arg in us)

  	esp_deep_sleep_start();
}