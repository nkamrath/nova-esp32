
#include <string.h>
#include "bluetooth_app.h"
#include "services/bluetooth.h"

#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "include/bluetooth_config.h"

#define GATTS_TAG "GATTS_DEMO"

extern void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

static uint8_t adv_service_uuid128[32] = BLUETOOTH_SERVICE_UUID;


static esp_ble_adv_data_t time_data = {
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
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t time_scan_data = {
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
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t descriptor_data = {
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
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_data_t descriptor_scan_data = {
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
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};


static prepare_type_env_t prepare_env;

static uint8_t last_data[256];
static uint8_t last_data_length = 0;

// static void _AppCallback1(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
// {
// 	ESP_LOGI(GATTS_TAG, "App Callback!\n");

// 	if(event == ESP_GATTS_READ_EVT)
// 	{
// 		ESP_LOGI(GATTS_TAG, "APP GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
//         esp_gatt_rsp_t rsp;
//         memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
//         rsp.attr_value.handle = param->read.handle;
//         rsp.attr_value.len = last_data_length;
//         memcpy(rsp.attr_value.value, last_data, last_data_length);
//         esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
//                                     ESP_GATT_OK, &rsp);
// 	}
// 	else if(event == ESP_GATTS_WRITE_EVT)
// 	{
// 		 ESP_LOGI(GATTS_TAG, "APP GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
//         ESP_LOGI(GATTS_TAG, "APP GATT_DATA: %s", param->write.value);
        
//         last_data_length = param->write.len;
//         memcpy(last_data, param->write.value, last_data_length);
        
//         Bluetooth_AppWrite(gatts_if, &prepare_env, param);
// 	}
// }

static void _TimeAppCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	ESP_LOGI(GATTS_TAG, "App Callback!\n");

	if(event == ESP_GATTS_READ_EVT)
	{
		ESP_LOGI(GATTS_TAG, "APP GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = 4;
        uint32_t ticks = xTaskGetTickCount()/100;
        memcpy(rsp.attr_value.value, &ticks, 4);
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
	}
}

static void _DescriptorAppCallback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	ESP_LOGI(GATTS_TAG, "Descriptor Callback!\n");

	if(event == ESP_GATTS_READ_EVT)
	{
		ESP_LOGI(GATTS_TAG, "APP GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = last_data_length;
        memcpy(rsp.attr_value.value, last_data, last_data_length);
        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp);
	}
	else if(event == ESP_GATTS_WRITE_EVT)
	{
		 ESP_LOGI(GATTS_TAG, "APP GATT_WRITE_EVT, conn_id %d, trans_id %d, handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        ESP_LOGI(GATTS_TAG, "APP GATT_DATA: %s", param->write.value);
        
        last_data_length = param->write.len;
        memcpy(last_data, param->write.value, last_data_length);
        
        Bluetooth_AppWrite(gatts_if, &prepare_env, param);
	}
}

void BluetoothTestApp_Create(void)
{
	bluetooth_app_registration_t time_app = {
		.low_level_callback = NULL,
		.app_callback = _TimeAppCallback,
		.service_uuid = 0x561B,
		.char_uuid = 0x0001
	};
	memcpy(&time_app.data, &time_data, sizeof(esp_ble_adv_data_t));
	memcpy(&time_app.scan_resp_data, &time_scan_data, sizeof(esp_ble_adv_data_t));
	Bluetooth_AddApp(&time_app);


	bluetooth_app_registration_t descriptor_app = {
		.low_level_callback = NULL,
		.app_callback = _DescriptorAppCallback,
		.service_uuid = 0x561B,
		.char_uuid = 0x0002
	};
	memcpy(&descriptor_app.data, &descriptor_data, sizeof(esp_ble_adv_data_t));
	memcpy(&descriptor_app.scan_resp_data, &descriptor_scan_data, sizeof(esp_ble_adv_data_t));
	Bluetooth_AddApp(&descriptor_app);
}