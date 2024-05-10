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
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
#include <FreeRTOSConfig.h>
#endif
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
const static char CONFIG_KEY_system_zone_id[] PROGMEM = {"system:zn_id"};
const static char CONFIG_KEY_system_zone_description[] PROGMEM = {"system:zn_dsc"};
const static char CONFIG_KEY_system_zone_name[] PROGMEM = {"system:zn_nam"};
const static char CONFIG_KEY_system_zone_offset[] PROGMEM = {"system:zn_ofs"};
const static char CONFIG_KEY_system_zone_lat[] PROGMEM = {"system:zn_lat"};
const static char CONFIG_KEY_system_zone_lng[] PROGMEM = {"system:zn_lng"};
const static char CONFIG_KEY_screen_rotation[] PROGMEM = {"screen:rotate"};

class ZoneConfig {
public:
    String id;
    String description;
    String name;
    int offset; // minutes
    float latitude;
    float longitude;
    ZoneConfig() {
        id = "UTC";
        description = "Coordinated Universal Time";
        name = "";
        offset = 0;            // UTC/GMT - Greenwich Mean Time
        latitude = 51.477928;  // Greenwich lat
        longitude = -0.001545; // Greenwich lng
    }
};

class SystemConfig {
public:
    String friendlyName;
    String systemMode;
    String ssid, pass;
    SystemConfig() {
        friendlyName = CONFIG_BUILTIN_MODULE_NAME;
        systemMode = "";
        ssid = "";
        pass = "";
    }
};

class Config {
public:
    const static short ServiceButtonPin = CONFIG_ServiceButtonPin;
    const static short StatusLedPin = CONFIG_StatusLedPin;
    const static uint16_t ConfigureButtonPushInterval = 2500;
    static ZoneConfig zone;
    static SystemConfig system;
#ifdef ESP32
    static ESP32Time* getRTC() {
        static ESP32Time rtc(0);
        return &rtc;
    }
#endif
    static void updateTimezone() {
        int tzo = zone.offset / 60;
        int tzm = zone.offset % 60;
        char tz[10] = "";
        sprintf(tz, "UTC%s%02d:%02d", tzo >= 0 ? "-" : "+", abs(tzo), abs(tzm));
        setenv("TZ", tz, 1);
        tzset();
    }

    static bool isDeviceConfigured() {
#ifdef CONFIGURE_WITH_WPS
        return !WiFi.SSID().isEmpty() && !WiFi.psk().isEmpty();
#else
        return !system.systemMode.equals("config") && !system.ssid.isEmpty() && !system.pass.isEmpty();
#endif
    }
    static bool isWiFiConfigured() {
#ifdef CONFIGURE_WITH_WPS
        return !WiFi.SSID().isEmpty() && !WiFi.psk().isEmpty();
#else
        return !system.ssid.isEmpty() && !system.pass.isEmpty();
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

    static std::function<void(bool)> ledCallback;
    static bool isStatusLedOn;
    static void statusLedOn();
    static void statusLedOff();

    static void statusLedCallback(std::function<void(bool)> new_func);

    static void init() {
        // Setup status led
        if (StatusLedPin >= 0) pinMode(StatusLedPin, OUTPUT);
#ifndef DISABLE_PREFERENCES
        Preferences preferences;

        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            // System and WiFi settings
            system.friendlyName = preferences.getString(CONFIG_KEY_device_name, system.friendlyName);
            system.systemMode = preferences.getString(CONFIG_KEY_system_mode, system.systemMode);
            system.ssid = preferences.getString(CONFIG_KEY_wifi_ssid, system.ssid);
            system.pass = preferences.getString(CONFIG_KEY_wifi_password, system.pass);
            // Time Zone
            zone.id = preferences.getString(CONFIG_KEY_system_zone_id, zone.id);
            zone.description = preferences.getString(CONFIG_KEY_system_zone_description, zone.description);
            zone.name = preferences.getString(CONFIG_KEY_system_zone_name, zone.name);
            zone.offset = preferences.getInt(CONFIG_KEY_system_zone_offset, zone.offset);
            zone.latitude = preferences.getFloat(CONFIG_KEY_system_zone_lat, zone.latitude);
            zone.longitude = preferences.getFloat(CONFIG_KEY_system_zone_lng, zone.longitude);
        } else {
            // initialize system preferences with default values
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putString(CONFIG_KEY_device_name, system.friendlyName);
            preferences.putString(CONFIG_KEY_system_mode, system.systemMode);
            preferences.putString(CONFIG_KEY_wifi_ssid, system.ssid);
            preferences.putString(CONFIG_KEY_wifi_password, system.pass);
            // Time Zone
            preferences.putString(CONFIG_KEY_system_zone_id, zone.id);
            preferences.putString(CONFIG_KEY_system_zone_description, zone.description);
            preferences.putString(CONFIG_KEY_system_zone_name, zone.name);
            preferences.putInt(CONFIG_KEY_system_zone_offset, zone.offset);
            preferences.putFloat(CONFIG_KEY_system_zone_lat, zone.latitude);
            preferences.putFloat(CONFIG_KEY_system_zone_lng, zone.longitude);
        }

        preferences.end();
#endif
        updateTimezone();
    }
};

#endif //HOMEGENIE_MINI_CONFIG_H
