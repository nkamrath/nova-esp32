#include "auth_app.h"

#include <string.h>
#include "bluetooth_app.h"
#include "services/bluetooth.h"

#include "include/bluetooth_config.h"

#include "utils/nvm_manager.h"
#include "esp_log.h"

static uint8_t service_uuid128[32] = BLUETOOTH_SERVICE_UUID;

static esp_ble_adv_data_t auth_app_data = {
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

static esp_ble_adv_data_t auth_app_scan_data = {
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


//static prepare_type_env_t prepare_env;

static void _ReadCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	if(event == ESP_GATTS_READ_EVT)
	{
        uint64_t auth_counter = NvmManager_GetCachedAuthCounter();
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 8;
        memcpy(rsp.attr_value.value, &auth_counter, 8);
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
	}
}

void AuthApp_Create(void)
{
	bluetooth_app_registration_t auth_app = {
		.low_level_callback = NULL,
		.app_callback = _ReadCallback,
		.service_uuid = BLUETOOTH_APP_SERVICE_UUID,
		.char_uuid = BLUETOOTH_APP_AUTH_TOKEN_CHAR_UUID
	};
	memcpy(&auth_app.data, &auth_app_data, sizeof(esp_ble_adv_data_t));
	memcpy(&auth_app.scan_resp_data, &auth_app_scan_data, sizeof(esp_ble_adv_data_t));
	Bluetooth_AddApp(&auth_app);
}