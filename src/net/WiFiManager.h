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

#ifndef HOMEGENIE_MINI_WIFIMANAGER_H
#define HOMEGENIE_MINI_WIFIMANAGER_H

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#include <Preferences.h>
#endif

#include <WiFiUdp.h>

#include <io/Logger.h>

#include "Config.h"
#include "Task.h"

#ifdef ESP32
#ifdef CONFIGURE_WITH_WPS
#include "esp_wifi.h"
#include "esp_wps.h"
#define ESP_WPS_MODE     WPS_TYPE_PBC
#define ESP_WPS_MANUFACTURER "G-Labs"
#define ESP_WPS_MODEL_NUMBER CONFIG_DEVICE_MODEL_NUMBER
#define ESP_WPS_MODEL_NAME   CONFIG_DEVICE_MODEL_NAME
#define ESP_WPS_DEVICE_NAME  CONFIG_SYSTEM_NAME
#endif
#endif

namespace Net {

    class WiFiManager : Task {
    public:
        WiFiManager();
        void loop() override;
        static void connect();
        static bool configure();
        static bool checkWiFiStatus();
    private:
        static wl_status_t wiFiStatus;
        static unsigned long lastStatusCheckTs;

#ifdef CONFIGURE_WITH_WPS
        static void setWiFiConfigured() {
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putString(CONFIG_KEY_wifi_ssid, WiFi.SSID());
            preferences.end();
        }
#ifdef ESP32
        static esp_wps_config_t wps_config;
        static bool esp32_wps_started;
        static bool wpsStart() {
            if (esp32_wps_started) return true;
            if (esp_wifi_wps_enable(&wps_config)) {
                // TODO: Serial.println("WPS Enable Failed");
                return false;
            } else if (esp_wifi_wps_start(0)) {
                // TODO: Serial.println("WPS Start Failed");
                return false;
            }
            esp32_wps_started = true;
            return true;
        }
        static void wpsStop() {
            if (esp_wifi_wps_disable()) {
                // TODO: Serial.println("WPS Disable Failed");
            }
            esp32_wps_started = false;
        }
        // WPS events handling
        static void WiFiEvent(WiFiEvent_t event, arduino_event_info_t info) {
            switch (event) {
                case ARDUINO_EVENT_WPS_ER_SUCCESS: {
                    wpsStop();
                    WiFi.begin();
                    connect();
                } break;
                case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
                    setWiFiConfigured();
                } break;
                case ARDUINO_EVENT_WPS_ER_FAILED:
                case ARDUINO_EVENT_WPS_ER_TIMEOUT:
                    wpsStop();
                    break;
                case ARDUINO_EVENT_WIFI_STA_START:
                case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                case ARDUINO_EVENT_WPS_ER_PIN:
                default:
                    break;
            }
        }
#else
        // WPS events handling
        static void WiFiEvent(WiFiEvent_t event) {
            switch(event) {
                case WIFI_EVENT_STAMODE_GOT_IP:
                case WIFI_EVENT_STAMODE_DISCONNECTED:
                case WIFI_EVENT_STAMODE_CONNECTED:
                    break;
                case WIFI_EVENT_STAMODE_AUTHMODE_CHANGE:
                    setWiFiConfigured();
                    break;
            }
        }
#endif // ESP32
#endif // CONFIGURE_WITH_WPS

    };

}
#endif //HOMEGENIE_MINI_WIFIMANAGER_H
