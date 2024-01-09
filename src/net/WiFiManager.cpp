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

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);
        wiFiStatus = WL_DISCONNECTED;
#ifdef ESP8266
        // WI-FI will not boot without this delay!!!
        delay(2000);
#endif
        connect();
    }

    void WiFiManager::loop() {
        auto status = WiFiClass::status();
        if (status != wiFiStatus) {
            wiFiStatus = status;
            checkWiFiStatus();
        }
    }

    void WiFiManager::connect() {
        IO::Logger::info("|  - Connecting to WI-FI .");
        // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
        WiFi.mode(WIFI_STA);
#ifdef CONFIGURE_WITH_WPA
        delay(1000); // TODO: is this delay necessary?
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
#else
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, true);
        String ssid = preferences.getString("wifi:ssid");
        String pass = preferences.getString("wifi:password");

        if (!ssid.isEmpty() && !pass.isEmpty()) {
            IO::Logger::info("|  - WI-FI SSID: %s", ssid.c_str());
            IO::Logger::info("|  - WI-FI Password: *"); // pass.c_str()
            WiFi.begin(ssid.c_str(), pass.c_str());
        }
        preferences.end();
#endif
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        auto status = WiFiClass::status();
        if (status == WL_CONNECTED) {
            digitalWrite(Config::StatusLedPin, LOW);
            IO::Logger::info("|  - Connected to '%s'", WiFi.SSID().c_str());
            IO::Logger::info("|  - IP: %s", WiFi.localIP().toString().c_str());
            wpsSuccess = true;
        } else {
            digitalWrite(Config::StatusLedPin, HIGH);
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
#ifdef CONFIGURE_WITH_WPA
#ifdef ESP8266
        // WPA currently only works for ESP8266
        digitalWrite(Config::StatusLedPin, LOW);
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        digitalWrite(Config::StatusLedPin, HIGH);
        IO::Logger::info ("|  >> Press WPS button on your router <<");
        bool wpsSuccess = WiFi.beginWPSConfig();
        return wpsSuccess;
#else
        // WPA only works with ESP8266
        return true;
#endif
#else
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);
        preferences.putString("wifi:ssid", "");
        preferences.putString("wifi:password", "");
        preferences.end();
        preferences.clear();
        IO::Logger::info("|  - WI-FI credentials reset!");
        delay(2000);
        IO::Logger::info("REBOOT!");
        // Reboot
        esp_restart();
        return true;
#endif
    }

}