#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_

#define BLUETOOTH_PROFILE_NUM 10
#define BLUETOOTH_DEVICE_NAME "Nova Control"

#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

typedef void (*bluetooth_low_level_callback_t)(uint8_t index, esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
typedef void (*bluetooth_app_callback_t)(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

typedef struct
{
	esp_ble_adv_data_t data;
	esp_ble_adv_data_t scan_resp_data;
	bluetooth_low_level_callback_t low_level_callback;
	bluetooth_app_callback_t app_callback;
	uint16_t service_uuid;
	uint16_t char_uuid;
} bluetooth_app_registration_t;

int Bluetooth_Create(void);
void Bluetooth_RegisterConnectCallback(void(*connect_callback)(void));
void Bluetooth_RegisterDisconnectCallback(void(*disconnect_callback)(void));
void Bluetooth_AddApp(bluetooth_app_registration_t* app_registration);
void Bluetooth_AppWrite(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

bool Bluetooth_HasConnection(void);

#endif