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
    wl_status_t WiFiManager::wiFiStatus = WL_DISCONNECTED;
    unsigned long WiFiManager::lastStatusCheckTs = 0;

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);
        // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
        WiFi.mode(WIFI_STA);
        WiFi.setAutoReconnect(true);
        if (Config::isDeviceConfigured()) {
#ifdef ESP8266
            // WI-FI will not boot without this delay!!!
            delay(2000);
#endif
            connect();
        }
    }

    void WiFiManager::loop() {
        if (!Config::isDeviceConfigured()) return;
        auto status = ESP_WIFI_STATUS;
        if (status != wiFiStatus || millis() - lastStatusCheckTs > 10000) {
            checkWiFiStatus();
            wiFiStatus = status;
            lastStatusCheckTs = millis();
        }
    }

    void WiFiManager::connect() {
        IO::Logger::info("|  - Connecting to WI-FI");
#ifdef CONFIGURE_WITH_WPS
        delay(1000); // TODO: is this delay necessary?
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
#else
        String ssid, pass;
        Preferences preferences;
        if (preferences.begin(CONFIG_SYSTEM_NAME, true)) {
            ssid = preferences.getString(CONFIG_KEY_wifi_ssid);
            pass = preferences.getString(CONFIG_KEY_wifi_password);
            preferences.end();
        }
        if (!ssid.isEmpty() && !pass.isEmpty()) {
            IO::Logger::info("|  - WI-FI SSID: \"%s\"", ssid.c_str());
            IO::Logger::info("|  - WI-FI Password: ***"); // pass.c_str()
            WiFi.begin(ssid.c_str(), pass.c_str());
        }
#endif
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        WiFi.waitForConnectResult(1500);
        auto status = ESP_WIFI_STATUS;
        if (status == WL_CONNECTED) {
            Config::statusLedOff();
            if (status != wiFiStatus) {
                IO::Logger::info("|  - Connected to '%s'", WiFi.SSID().c_str());
                IO::Logger::info("|  - IP: %s", WiFi.localIP().toString().c_str());
            }
            wpsSuccess = true;
        } else {
            Config::statusLedOn();
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
                    break;
            }
        }
        return wpsSuccess;
    }

    bool WiFiManager::configure() {
#ifdef CONFIGURE_WITH_WPS
#ifdef ESP8266
        // WPS currently only works for ESP8266
        Config::statusLedOff();
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        Config::statusLedOn();
        IO::Logger::info ("|  >> Press WPS button on your router <<");
        bool wpsSuccess = WiFi.beginWPSConfig();
        return wpsSuccess;
#else
        // WPS only works with ESP8266
        IO::Logger::error("|  x WPS only works with ESP8266");
        return false;
#endif
#else
    #ifndef DISABLE_PREFERENCES
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString(CONFIG_KEY_wifi_ssid, "");
        preferences.putString(CONFIG_KEY_wifi_password, "");
        preferences.end();
        preferences.clear();
        IO::Logger::info("|  - WI-FI credentials reset!");
        delay(2000);
        IO::Logger::info("REBOOT!");
        // Reboot
        esp_restart();
        return true;
    #endif
#endif
    }

}