// Copyright 2015-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/****************************************************************************
*
* This file is for gatt server. It can send adv data, be connected by clent.
* Run the gatt_client demo, the client demo will automatically connect to the gatt_server demo.
* Client demo will enable gatt_server's notify after connection. Then two devices will exchange
* data.
*
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_wifi_types.h"
#include "esp_wifi.h"

#include "sdkconfig.h"

#include "services/bluetooth.h"
#include "bluetooth_app.h"
#include "apps/auth_app.h"
#include "utils/nvm_manager.h"
#include "utils/command_handler.h"
#include "apps/dio_control.h"
#include "services/io_controller.h"
#include "apps/power_manager.h"
#include "utils/neopixel.h"

void app_main()
{

    NvmManager_Create();

    //turn off wifi
    esp_wifi_set_mode(WIFI_MODE_NULL);

    Bluetooth_Create();
    BluetoothTestApp_Create();
    AuthApp_Create();
    CommandHandler_Create();
    IoController_Create();
    IoController_Read(0);
    IoController_Read(0);

    neopixel_t pixels[12];
    for(int i = 0; i < 12; i++){
        Neopixel_Create(60, i, 0, &pixels[i]);
    }

    DioControl_Create();

    PowerManager_Create();

    bool increase = true;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    //sleep handling
    while(1)
    {
        vTaskDelay(5);

        if(increase)
        {
            r += 1;
            //g += 1;
            //b += 1;
        }
        else
        {
            r -= 1;
            //g -= 1;
            //b -= 1;
        }

        if(r >= 32)
        {
            increase = false;
        }
        else if(r == 0)
        {
            increase = true;
        }

        for(int i = 0; i < 12; i++)
        {
            Neopixel_SetRgb(&pixels[i], ((r<<16) | (g<<8) | b));
        }

        // uint32_t last_rgb = pixels[11].rgb;
        // for(int i = 11; i > 0; i--)
        // {
        //     Neopixel_SetRgb(&pixels[i], pixels[i-1].rgb);
        // }
        // Neopixel_SetRgb(&pixels[0], last_rgb);
    }
}
