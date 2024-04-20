/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
 *
 *
 * This file is part of HomeGenie-Mini (HGM).
 *
 *  HomeGenie-Mini is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  HomeGenie-Mini is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with HomeGenie-Mini.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Authors:
 * - Generoso Martello <gene@homegenie.it>
 *
 *
 */

#ifndef HOMEGENIE_MINI_DEFS_H
#define HOMEGENIE_MINI_DEFS_H

#define CONFIG_SYSTEM_NAME                 "hg-mini"

#define CONFIG_DEVICE_MODEL_NAME           "HomeGenie Mini"
#define CONFIG_DEVICE_MODEL_NUMBER         "1.2.0"
#define CONFIG_DEVICE_SERIAL_NUMBER        "ABC0123456789"

#define CONFIG_BUILTIN_MODULE_NAME         "HG-Mini"
#define CONFIG_BUILTIN_MODULE_ADDRESS      "mini"

#define DEBUGLOG_DEFAULT_LOG_LEVEL_ERROR

#define CONFIG_CREATE_AUTOMATION_TASK

#ifdef ESP8266
#define ESP_WIFI_STATUS WiFi.status()
#define FILE_READ       "r"
#define FILE_WRITE      "w"
#define FILE_APPEND     "a"
#else
#define ESP_WIFI_STATUS WiFiClass::status()
#endif

#ifdef ESP8266
    #define DISABLE_AUTOMATION
    #define DISABLE_UI
    #define DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
    #define DISABLE_PREFERENCES
    #define CONFIGURE_WITH_WPS
    #define WebServer ESP8266WebServer
    #ifndef CONFIG_GPIO_OUT
        #define CONFIG_GPIO_OUT {14,12,13,15}
    #endif
#endif

#ifdef ESP32_C3
    #undef DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
    #define CONFIG_ServiceButtonPin 4
    #if CONFIG_StatusLedPin != -1
    #define CONFIG_StatusLedPin 0
    #endif
    #ifndef CONFIG_GPIO_OUT
        #define CONFIG_GPIO_OUT {6}
    #endif
#elif ESP32_S3
    #undef DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
    #define CONFIG_ServiceButtonPin 21
    #if CONFIG_StatusLedPin != -1
    #define CONFIG_StatusLedPin 33
    #endif
    #ifndef CONFIG_GPIO_OUT
        #define CONFIG_GPIO_OUT {15,16,17,18}
    #endif
#else
    #define DISABLE_BLUETOOTH_LE
//    #define DISABLE_BLUETOOTH_CLASSIC
//    #define DISABLE_MQTT
//    #undef DISABLE_BLUETOOTH_LE
//    #undef DISABLE_UI
#endif

#ifndef CONFIG_ServiceButtonPin
    #define CONFIG_ServiceButtonPin 2
#endif
#ifndef CONFIG_StatusLedPin
    #define CONFIG_StatusLedPin 16
#endif
#ifndef CONFIG_GPIO_OUT
    #define CONFIG_GPIO_OUT {14,12,13,17}
#endif
#ifndef CONFIG_GPIO_IN
    #define CONFIG_GPIO_IN { /* not implemented */ }
#endif


// --------------------------------


#ifdef ESP32_S3

#define CONFIG_DISPLAY_SCLK 10
#define CONFIG_DISPLAY_MOSI 11
#define CONFIG_DISPLAY_MISO 12
#define CONFIG_DISPLAY_RST 14

#define CONFIG_DISPLAY_DC 8
#define CONFIG_DISPLAY_CS 9

#define CONFIG_DISPLAY_BL (2) // backlight

#define CONFIG_TOUCH_PORT 1
#define CONFIG_TOUCH_ADDRESS 0x15

#define CONFIG_TOUCH_INT (-1) /* -1 disabled, or pin 5 */
#define CONFIG_TOUCH_SDA 6
#define CONFIG_TOUCH_SCL 7
#define CONFIG_TOUCH_RST 13

#else

#define CONFIG_DISPLAY_SCLK 18
#define CONFIG_DISPLAY_MOSI 23
#define CONFIG_DISPLAY_MISO 19
#define CONFIG_DISPLAY_RST 33

#define CONFIG_DISPLAY_DC 27
#define CONFIG_DISPLAY_CS 14

#define CONFIG_DISPLAY_BL (32) // backlight

#define CONFIG_TOUCH_PORT 1
#define CONFIG_TOUCH_ADDRESS 0x15

#define CONFIG_TOUCH_INT (-1) /* -1 disabled, or pin 35 */
#define CONFIG_TOUCH_SDA 25
#define CONFIG_TOUCH_SCL 26
#define CONFIG_TOUCH_RST 34

#endif



#endif // HOMEGENIE_MINI_DEFS_H
