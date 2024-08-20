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

#include "version.h"

#define CONFIG_SYSTEM_NAME                 "hg-mini"

#define CONFIG_DEVICE_MODEL_NAME           "HomeGenie Mini"
#define CONFIG_DEVICE_MODEL_NUMBER         STRING_VALUE(VERSION_MAJOR) "." STRING_VALUE(VERSION_MINOR) "." STRING_VALUE(VERSION_PATCH)
#define CONFIG_DEVICE_SERIAL_NUMBER        "ABC0123456789"

#define CONFIG_BUILTIN_MODULE_NAME         "HG-Mini"
#define CONFIG_BUILTIN_MODULE_ADDRESS      "mini"

#define DEBUGLOG_DEFAULT_LOG_LEVEL_ERROR

#ifndef BUILD_ENV_NAME
    #define BUILD_ENV_NAME "default-env"
#endif


// Automatically enable FreeRTOS task on devices with additional RAM
// (is disabled otherwise saving about ~12K or RAM)
//#ifdef BOARD_HAS_PSRAM
// TODO: experimental flag (rc issues tbi)
//#define CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
//#endif

// disabling SSE and MQTT saves only ~2K of RAM
//#define DISABLE_SSE
//#define DISABLE_MQTT

// WPS active for all configurations by default
#define CONFIGURE_WITH_WPS

#ifdef ESP8266
    #define ESP_WIFI_STATUS WiFi.status()
    #define FILE_READ       "r"
    #define FILE_WRITE      "w"
    #define FILE_APPEND     "a"
#else
    #define ESP_WIFI_STATUS WiFiClass::status()
#endif

#ifdef ESP8266
    #undef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
    #define DISABLE_UI
    #define DISABLE_BLUETOOTH
    #define WebServer ESP8266WebServer
#endif


#ifdef ESP32_C3
    #undef DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
#elif ESP32_S2
    #define DISABLE_BLUETOOTH
#elif ESP32_S3
    #undef DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
#else
    #define DISABLE_BLUETOOTH_LE
#endif



#ifdef DISABLE_BLUETOOTH
    #define DISABLE_BLUETOOTH_LE
    #define DISABLE_BLUETOOTH_CLASSIC
#endif



// ---------------- [ DEFAULT DISPLAY CONFIG ] ----------------


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
