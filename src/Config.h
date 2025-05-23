/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

#include <Arduino.h>
#include <ArduinoJson.h>

#include "defs.h"

#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
#include <FreeRTOSConfig.h>
#endif

#ifdef ESP32
#include <ESP32Time.h>
#ifdef CONFIGURE_WITH_WPS
#include "esp_wifi.h"
#endif
#endif

#ifdef CONFIGURE_WITH_WPS
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#endif

#include <Preferences.h>

const static char CONFIG_KEY_wifi_ssid[] = {"wifi:ssid"};
const static char CONFIG_KEY_wifi_password[] = {"wifi:password"};
const static char CONFIG_KEY_device_name[] = {"device:name"};
const static char CONFIG_KEY_device_group[] = {"device:group"};
const static char CONFIG_KEY_system_mode[] = {"system:mode"};
const static char CONFIG_KEY_system_zone_id[] = {"system:zn_id"};
const static char CONFIG_KEY_system_zone_description[] = {"system:zn_dsc"};
const static char CONFIG_KEY_system_zone_name[] = {"system:zn_nam"};
const static char CONFIG_KEY_system_zone_offset[] = {"system:zn_ofs"};
const static char CONFIG_KEY_system_zone_lat[] = {"system:zn_lat"};
const static char CONFIG_KEY_system_zone_lng[] = {"system:zn_lng"};
const static char CONFIG_KEY_system_ntp_server[] = {"system:ntpsrv"};
const static char CONFIG_KEY_screen_rotation[] = {"screen:rotate"};

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
    String id;
    String friendlyName;
    String systemMode;
    String ssid, pass;
    String ntpServer;

    SystemConfig() {
        id = "";
        friendlyName = CONFIG_BUILTIN_MODULE_NAME;
        systemMode = "";
        ssid = "";
        pass = "";
        ntpServer = "pool.ntp.org";
    }
};

class Config {
public:
    static short ServiceButtonPin;
    const static uint16_t ConfigureButtonPushInterval = 3000;
    static short StatusLedPin;
    static bool StatusLedInvert;
    static bool isStatusLedOn;
    static bool wpsRequest;
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
        return isWiFiConfigured();
    }

    static bool isWiFiConfigured() {
        return !system.systemMode.equals("config") && !system.ssid.isEmpty();
    }
    static void onWiFiConfigured() {
        // reset system mode if it was previously forced to "config"
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString(CONFIG_KEY_system_mode, "");
        preferences.end();
        // invoke callback if set
        if (wifiConfiguredCallback != nullptr) {
            wifiConfiguredCallback();
        }
    }
    static void setOnWiFiConfiguredCallback(std::function<void()> callback) {
        wifiConfiguredCallback = std::move(callback);
    }

    static bool saveSetting(const char* key, String value) {
        String k = String("$") + String(key);
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString(k.c_str(), value);
        preferences.end();
        return false; // TODO: ?!? return *true* if saved otherwise *false*
    }

    static String getSetting(const char* key, const char* defaultValue = "") {
        String value = defaultValue;
        // key length is limited to 15 chars.
        String k = String("$") + String(key);
        Preferences preferences;
        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            value = preferences.getString(k.c_str(), defaultValue);
        }
        preferences.end();
        // TODO: should use "pref.isKey(...)" for proper checking
        if (value == "") {
            // Lookup config factory defaults only if the value does not exist
            String config = STRING_VALUE(DEFAULT_CONFIG);
            if (!config.isEmpty()) {
#ifdef BOARD_HAS_PSRAM
                struct SpiRamAllocator : ArduinoJson::Allocator {
                    void* allocate(size_t size) override {
                        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
                    }

                    void deallocate(void* pointer) override {
                        heap_caps_free(pointer);
                    }

                    void* reallocate(void* ptr, size_t new_size) override {
                        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
                    }
                };
                SpiRamAllocator allocator;
                JsonDocument doc(&allocator);
#else
                JsonDocument doc;
#endif
                DeserializationError error = deserializeJson(doc, config);
                if (error.code() == 0) {
                    auto params = doc.as<JsonObject>();
                    for (auto p: params) {
                        if (p.key() == key) {
                            value = p.value().as<String>();
                            break;
                        }
                    }
                }
            }
        }
        return value;
    }
    static const char* getKey(const char* key, int i) {
        String configKey = key;
        if (i > 0) {
            configKey = configKey + String("#") + String(i);
        }
        char* k = new char[configKey.length() + 1];
        configKey.toCharArray(k, configKey.length() + 1);
        return k;
    }

    static void statusLedOn();
    static void statusLedOff();

    static void statusLedCallback(std::function<void(bool)> new_func);

    static void init() {
        // Setup status LED and factory reset buttons
        ServiceButtonPin = getSetting("sys-rb-n").toInt();
        StatusLedPin = getSetting("sys-sl-n").toInt();
        // Setup status LED
        if (StatusLedPin >= 0) pinMode(StatusLedPin, OUTPUT);
        Preferences preferences;

        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            // System and WiFi settings
            system.friendlyName = preferences.getString(CONFIG_KEY_device_name, system.friendlyName);
            system.systemMode = preferences.getString(CONFIG_KEY_system_mode, system.systemMode);
            system.ssid = preferences.getString(CONFIG_KEY_wifi_ssid, system.ssid);
            system.pass = preferences.getString(CONFIG_KEY_wifi_password, system.pass);
            system.ntpServer = preferences.getString(CONFIG_KEY_system_ntp_server, system.ntpServer);
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
            preferences.putString(CONFIG_KEY_system_ntp_server, system.ntpServer);
            // Time Zone
            preferences.putString(CONFIG_KEY_system_zone_id, zone.id);
            preferences.putString(CONFIG_KEY_system_zone_description, zone.description);
            preferences.putString(CONFIG_KEY_system_zone_name, zone.name);
            preferences.putInt(CONFIG_KEY_system_zone_offset, zone.offset);
            preferences.putFloat(CONFIG_KEY_system_zone_lat, zone.latitude);
            preferences.putFloat(CONFIG_KEY_system_zone_lng, zone.longitude);
        }

        preferences.end();
        updateTimezone();
    }

    static void handleConfigCommand(String &message);
private:
    static std::function<void(bool)> ledCallback;
    static std::function<void()> wifiConfiguredCallback;
};

#endif //HOMEGENIE_MINI_CONFIG_H
