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
//#include <wifi_provisioning/manager.h>
//#include <qrcode.h>

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
#ifdef ESP32
        static bool wpsIsRegistering() {
            return esp32_wps_waiting_connect_ts > 0;
        }
        static bool wpsIsTimedOut() {
            // TODO: add constant WPS_REGISTER_TIMEOUT = 15000
            return wpsIsRegistering() && (millis() - esp32_wps_waiting_connect_ts > 15000);
        }
        static void wpsCancel() {
            wpsStop();
            Config::statusLedOff();
        }
#endif
    private:
        static wl_status_t wiFiStatus;
        static unsigned long lastStatusCheckTs;
#ifdef ESP32
        static unsigned long esp32_wps_waiting_connect_ts;
#endif

#ifdef CONFIGURE_WITH_WPS
        static void setWiFiConfigured() {
            bool wasConfigured = Config::isWiFiConfigured();
            Preferences preferences;
            preferences.begin(CONFIG_SYSTEM_NAME, false);
            preferences.putString(CONFIG_KEY_wifi_ssid, WiFi.SSID());
            preferences.end();
            if (!wasConfigured) {
                Config::onWiFiConfigured();
            }
            /*

            // This is  just a test to see if DPP support compiles on
            // current SDK version (and yes, it does since espressif32@6.7.0)

            char payload[150] = {0};
            snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\"" \
                    ",\"transport\":\"%s\"}",
                     "v1", "MyAP", "softap");

            esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
            esp_qrcode_generate(&cfg, payload);
            Serial.println(payload);

            // official docs page https://github.com/espressif/esp-idf/tree/5ca9f2a4/examples/wifi/wifi_easy_connect/dpp-enrollee

            */
        }
#ifdef ESP8266
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
#else // ESP32
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
            esp32_wps_waiting_connect_ts = 0;
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
                    esp32_wps_waiting_connect_ts = millis();
                    Config::statusLedOn();
                    delay(2000); // without this delay the connection might fail
                    connect();
                } break;
                case ARDUINO_EVENT_WIFI_STA_GOT_IP: {
                    setWiFiConfigured();
                    esp32_wps_waiting_connect_ts = 0;
                    Config::statusLedOff();
                } break;
                case ARDUINO_EVENT_WPS_ER_FAILED:
                case ARDUINO_EVENT_WPS_ER_TIMEOUT: {
                    wpsStop();
                    esp32_wps_waiting_connect_ts = 0;
                } break;
                case ARDUINO_EVENT_WIFI_STA_START:
                case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
                case ARDUINO_EVENT_WPS_ER_PIN:
                default:
                    break;
            }
        }
#endif
#endif // CONFIGURE_WITH_WPS

    };

}
#endif //HOMEGENIE_MINI_WIFIMANAGER_H
