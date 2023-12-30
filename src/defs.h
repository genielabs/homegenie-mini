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

#define CONFIG_DEVICE_MODEL_NAME         "HomeGenie Mini"
#define CONFIG_DEVICE_MODEL_NUMBER       "1.2.0"
#define CONFIG_DEVICE_SERIAL_NUMBER      "ABC0123456789"

#define CONFIG_BUILTIN_MODULE_NAME         "HG-Mini"
#define CONFIG_BUILTIN_MODULE_ADDRESS      "mini"

#define DISABLE_BLE

#ifdef ESP8266
    #define CONFIGURE_WITH_WPA
    #define WebServer ESP8266WebServer
    #ifndef CONFIG_GPIO_OUT
        #define CONFIG_GPIO_OUT {14,12,13,15}
    #endif
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

#endif // HOMEGENIE_MINI_DEFS_H
