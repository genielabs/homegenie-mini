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
 * Releases:
 * - 2019-02-16 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_CONFIG_H
#define HOMEGENIE_MINI_CONFIG_H

#include "defs.h"

#include <Arduino.h>
#ifdef ESP32
#include <ESP32Time.h>
#else
#ifdef CONFIGURE_WITH_WPS
#include <ESP8266WiFi.h>
#endif
#endif
#ifndef DISABLE_PREFERENCES
#include <Preferences.h>
#endif

const static char CONFIG_KEY_wifi_ssid[] PROGMEM = {"wifi:ssid"};
const static char CONFIG_KEY_wifi_password[] PROGMEM = {"wifi:password"};
const static char CONFIG_KEY_device_name[] PROGMEM = {"device:name"};
const static char CONFIG_KEY_system_mode[] PROGMEM = {"system:mode"};
const static char CONFIG_KEY_screen_rotation[] PROGMEM = {"screen:rotation"};

class Config {
public:
    const static uint8_t ServiceButtonPin = CONFIG_ServiceButtonPin;
    const static uint8_t StatusLedPin = CONFIG_StatusLedPin;
    const static uint16_t WpsModePushInterval = 2500;
#ifdef ESP32
    static ESP32Time* getRTC() {
        static ESP32Time rtc(0);
        return &rtc;
    }
#endif

    static bool isDeviceConfigured() {
#ifdef CONFIGURE_WITH_WPS
        return !WiFi.SSID().isEmpty() && !WiFi.psk().isEmpty();
#else
#ifndef DISABLE_PREFERENCES
        String systemMode;
        String ssid, pass;
        Preferences preferences;
        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            systemMode = preferences.getString(CONFIG_KEY_system_mode);
            ssid = preferences.getString(CONFIG_KEY_wifi_ssid);
            pass = preferences.getString(CONFIG_KEY_wifi_password);
        } else {
            // initialize system preferences with default values
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putString(CONFIG_KEY_system_mode, "");
            preferences.putString(CONFIG_KEY_wifi_ssid, "");
            preferences.putString(CONFIG_KEY_wifi_password, "");
        }
        preferences.end();
        return !systemMode.equals("config") && !ssid.isEmpty() && !pass.isEmpty();
#endif
#endif
        return false;
    }
    static bool isWiFiConfigured() {
#ifdef CONFIGURE_WITH_WPS
        return !WiFi.SSID().isEmpty() && !WiFi.psk().isEmpty();
#else
#ifndef DISABLE_PREFERENCES
        String ssid, pass;
        Preferences preferences;
        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            ssid = preferences.getString(CONFIG_KEY_wifi_ssid);
            pass = preferences.getString(CONFIG_KEY_wifi_password);
        }
        preferences.end();
        return !ssid.isEmpty() && !pass.isEmpty();
#endif
#endif
        return false;
    }

    static String getDeviceName() {
#ifndef DISABLE_PREFERENCES
        if (isDeviceConfigured()) {
            // Read friendly name from prefs
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, true);
            String friendlyName = preferences.getString(CONFIG_KEY_device_name, CONFIG_BUILTIN_MODULE_NAME);
            preferences.end();
            return friendlyName;
        } else {
            return CONFIG_BUILTIN_MODULE_NAME;
        }
#else
        return CONFIG_BUILTIN_MODULE_NAME;
#endif
    }

    static bool saveSetting(const char* key, String& value) {
#ifndef DISABLE_PREFERENCES
        String k = String("$") + String(key);
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString(k.c_str(), value);
        preferences.end();
#endif
        return false;
    }

    static String getSetting(const char* key) {
        String value;
#ifndef DISABLE_PREFERENCES
        String k = String("$") + String(key);
        Preferences preferences;
        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            value = preferences.getString(k.c_str());
        }
        preferences.end();
#endif
        return value;
    }

};

#endif //HOMEGENIE_MINI_CONFIG_H
