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

#include "WiFiManager.h"

namespace Net {
    wl_status_t WiFiManager::wiFiStatus = WL_NO_SSID_AVAIL;
    unsigned long WiFiManager::wiFiIdleTimestamp = 0;
#ifdef ESP32
    esp_wps_config_t WiFiManager::wps_config;
    bool WiFiManager::esp32_wps_started = false;
    unsigned long WiFiManager::esp32_wps_waiting_connect_ts = 0;
#endif

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);

#ifdef ESP8266
        WiFi.mode(WIFI_STA);
        //WiFi.setOutputPower(20.5);
#else
        WiFiClass::mode(WIFI_STA);
        //WiFi.setTxPower(WIFI_POWER_19_5dBm);
#endif
        WiFi.setAutoReconnect(true);

#ifdef CONFIGURE_WITH_WPS
#ifdef ESP32
        wps_config.wps_type = ESP_WPS_MODE;
        strcpy(wps_config.factory_info.manufacturer, ESP_WPS_MANUFACTURER);
        strcpy(wps_config.factory_info.model_number, ESP_WPS_MODEL_NUMBER);
        strcpy(wps_config.factory_info.model_name, ESP_WPS_MODEL_NAME);
        strcpy(wps_config.factory_info.device_name, ESP_WPS_DEVICE_NAME);
#endif
        WiFi.onEvent(WiFiEvent);
#endif

        if (Config::isDeviceConfigured()) {
#ifdef ESP8266
            // WI-FI will not boot without this delay!!!
            delay(2000);
#endif
        }
    }

    void WiFiManager::loop() {
        if (!Config::isDeviceConfigured()) return;
        auto status = ESP_WIFI_STATUS;
        if (status != wiFiStatus) {

            // work-around for re-connection issue on version 6.9.0
            if (status != WL_CONNECTED) {
                WiFi.disconnect();
            }

            checkWiFiStatus();
            wiFiStatus = ESP_WIFI_STATUS;
            if (wiFiStatus == WL_DISCONNECTED) {
                wiFiStatus = WL_NO_SSID_AVAIL;
            }
        } else if (wiFiStatus == WL_IDLE_STATUS) {
            if (wiFiIdleTimestamp == 0) {
                wiFiIdleTimestamp = millis();
            } else if (millis() - wiFiIdleTimestamp > 15000) {
                // force reconnect
                wiFiStatus = WL_NO_SSID_AVAIL;
                wiFiIdleTimestamp = 0;
                connect();
            }
        }
    }

    void WiFiManager::connect() {
        Config::statusLedOn();
        IO::Logger::info("|  - Connecting to WI-FI");
        // If stored password is empty use WPS credentials if present
        if (Config::system.pass.isEmpty()) {
            delay(1000); // TODO: is this delay necessary?
#ifdef ESP8266
            WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
            WiFi.waitForConnectResult(10000);
#else
            wifi_config_t config;
            esp_err_t err = esp_wifi_get_config(WIFI_IF_STA, &config);
            if (err == ESP_OK) {

                WiFi.begin((char *) config.sta.ssid, (char *) config.sta.password);
                WiFi.waitForConnectResult(10000);

                esp32_wps_waiting_connect_ts = 0;

            } else {
                // TODO: Serial.printf("Couldn't get config: %d\n", (int) err);
            }
#endif
        } else if (!Config::system.ssid.isEmpty() && !Config::system.pass.isEmpty()) {
            IO::Logger::info("|  - WI-FI SSID: \"%s\"", Config::system.ssid.c_str());
            IO::Logger::info("|  - WI-FI Password: ***"); // Config::system.pass.c_str()
            WiFi.begin(Config::system.ssid.c_str(), Config::system.pass.c_str());
            WiFi.waitForConnectResult(10000);
        }
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        auto status = ESP_WIFI_STATUS;
        if (status == WL_CONNECTED) {
            Config::statusLedOff();
            if (status != wiFiStatus) {
                IO::Logger::info("|  - Connected to '%s'", WiFi.SSID().c_str());
                IO::Logger::info("|  - IP: %s", WiFi.localIP().toString().c_str());
            }
            wpsSuccess = true;
        } else {
            switch (status) {
                case WL_NO_SSID_AVAIL:
                    IO::Logger::error("|  x WiFi SSID not available");
                    connect();
                    break;
                case WL_CONNECT_FAILED:
                    IO::Logger::error("|  x WiFi connection failed");
                    connect();
                    break;
                case WL_CONNECTION_LOST:
                    IO::Logger::error("|  x WiFi connection lost");
                    connect();
                    break;
                case WL_DISCONNECTED:
                    IO::Logger::error("|  x WiFi disconnected");
                    connect();
                    break;
                case WL_NO_SHIELD:
                    IO::Logger::error("|  x WiFi initialization failed");
                    break;
                case WL_SCAN_COMPLETED:
                    IO::Logger::error("|  x Not connected to WiFi (state='%d')", status);
                    break;
                case WL_IDLE_STATUS:
                    IO::Logger::error("|  x WiFi idle (?)");
                    break;
            }
        }
        return wpsSuccess;
    }

    bool WiFiManager::configure() {
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString(CONFIG_KEY_wifi_ssid, "");
        preferences.putString(CONFIG_KEY_wifi_password, "");
        preferences.end();
        preferences.clear();
#ifdef ESP8266
        ESP.eraseConfig();
#else
        // TODO: find a way to clear wi-fi credentials on ESP32 the
        //       following commented code was apparently not working
//        wifi_config_t wifi_cfg_empty;
//        memset(&wifi_cfg_empty, 0, sizeof(wifi_config_t));
//        esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg_empty);
#endif
        IO::Logger::info("|  - WI-FI config reset!");
#ifdef CONFIGURE_WITH_WPS
        Config::statusLedOff();
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        Config::statusLedOn();
        IO::Logger::info ("|  >> Press WPS button on your router <<");
#ifdef ESP8266
        return WiFi.beginWPSConfig();
#else
        int timeout = 400;
        bool success = wpsStart();
        while (esp32_wps_started && timeout-- > 0) {
            delay(100);
            yield();
        }
        wpsStop();
        return success;
#endif
#else
        delay(2000);
        IO::Logger::info("RESTART!");
        // Restart device
        ESP.restart();
        return true;
#endif
    }

}