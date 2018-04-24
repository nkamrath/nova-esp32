#include "command_handler.h"

#include <string.h>
#include "bluetooth_app.h"
#include "services/bluetooth.h"

#include "include/bluetooth_config.h"
#include "utils/nvm_manager.h"

#include "hwcrypto/sha.h"
#include "esp_log.h"

#define _MAX_COMMAND_CALLBACKS	64


typedef struct  __attribute__((packed))
{
	uint8_t type;
	uint8_t length;
	uint32_t authentication_token;
	uint8_t data[];
} _bluetooth_command_t;

typedef struct
{
	uint8_t type;
	bluetooth_command_callback_t callback;
} _callback_registration_t;

static uint8_t service_uuid128[32] = BLUETOOTH_SERVICE_UUID;
static uint8_t _auth_buffer[MAX_AUTH_ARRAY_SIZE];

static esp_ble_adv_data_t command_app_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 32,
    .p_service_uuid = service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t command_app_scan_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x20,
    .max_interval = 0x40,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = 32,
    .p_service_uuid = service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static bool _initialized = false;
static prepare_type_env_t prepare_env;
static _callback_registration_t _command_callbacks[_MAX_COMMAND_CALLBACKS];

bool _ValidateCommand(_bluetooth_command_t* command)
{

	ESP_LOGE("COMMAND", "type: %d\n", command->type);
	ESP_LOGE("COMMAND", "length: %d\n", command->length);
	ESP_LOGE("COMMAND", "token: %d\n", command->authentication_token);

	if(command->length > MAX_BLUETOOTH_COMMAND_DATA_SIZE)
	{
		//invalid packet length
		ESP_LOGE("COMMAND", "invalid length\n");
		return false;
	}

	//need to build the array to validate auth token against
	uint8_t index = 0;
	memcpy(_auth_buffer, command->data, command->length);
	index += command->length;

	uint64_t auth_counter = NvmManager_GetCachedAuthCounter();
	NvmManager_IncrementAuthCounter();
	memcpy(&_auth_buffer[index], &auth_counter, sizeof(auth_counter));
	index += sizeof(auth_counter);

	memcpy(&_auth_buffer[index], BLUETOOTH_AUTH_SECRET_KEY, BLUETOOTH_AUTH_SECRET_KEY_SIZE);
	index += BLUETOOTH_AUTH_SECRET_KEY_SIZE;

	//uint8_t password_length = NvmManager_GetPassword((void*)&_auth_buffer[index]);
	//index += password_length;

	uint8_t computed_sha[32];
	memset(computed_sha, 0, sizeof(computed_sha));
	esp_sha(SHA2_256, (void*)_auth_buffer, index, computed_sha);
	uint32_t new_token = (computed_sha[3] << 24) | (computed_sha[2] << 16) | (computed_sha[1] << 8) | (computed_sha[0]);

	if(new_token == command->authentication_token)
	{
		//good packet
		//ESP_LOGE("COMMAND", "good command\n");
		return true;
	}
	ESP_LOGE("COMMAND", "bad command\n");
	ESP_LOGE("COMMAND", "computed token %d\n", new_token);
	return false;
}

static void _RunCallbacks(_bluetooth_command_t* command)
{
	for(int i = 0; i < _MAX_COMMAND_CALLBACKS; i++)
	{
		if(_command_callbacks[i].type == command->type)
		{
			_command_callbacks[i].callback(command->data, command->length);
		}
	}
}

static void _HandleCommand(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	if(event == ESP_GATTS_WRITE_EVT)
	{
		ESP_LOGE("COMMAND", "write\n");
		_bluetooth_command_t* command = (_bluetooth_command_t*)param->write.value;
		bool result = _ValidateCommand(command);
		if(result)
		{
			//process the command only if it was considered valid (fields were all validated and the auth token checked out)
			_RunCallbacks(command);
		}
        Bluetooth_AppWrite(gatts_if, &prepare_env, param);
	}
}

void CommandHandler_Create(void)
{
	if(!_initialized)
	{
		for(int i = 0; i < _MAX_COMMAND_CALLBACKS; i++)
		{
			_command_callbacks[i].type = 0;
			_command_callbacks[i].callback = NULL;
		}

		bluetooth_app_registration_t command_app = {
			.low_level_callback = NULL,
			.app_callback = _HandleCommand,
			.service_uuid = BLUETOOTH_APP_SERVICE_UUID,
			.char_uuid = BLUETOOTH_APP_COMMAND_HANDLER_CHAR_UUID
		};
		memcpy(&command_app.data, &command_app_data, sizeof(esp_ble_adv_data_t));
		memcpy(&command_app.scan_resp_data, &command_app_scan_data, sizeof(esp_ble_adv_data_t));
		Bluetooth_Create();
		Bluetooth_AddApp(&command_app);
		_initialized = true;
	}
}

void CommandHandler_RegisterCallback(uint8_t type, bluetooth_command_callback_t callback)
{
	for(int i = 0; i < _MAX_COMMAND_CALLBACKS; i++)
	{
		if(_command_callbacks[i].callback == NULL)
		{
			_command_callbacks[i].type = type;
			_command_callbacks[i].callback = callback;
			return;
		}
	}
}