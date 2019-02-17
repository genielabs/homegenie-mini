/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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

#include <Config.h>
#include "WiFiManager.h"

namespace Net {

    WiFiManager::WiFiManager() {
        setLoopInterval(1000);
        wiFiStatus = WL_DISCONNECTED;
        // WI-FI will not boot without this delay!!!
        delay(2000);
        initWiFi();
    }

    void WiFiManager::loop() {
        wl_status_t status = WiFi.status();
        if (status != wiFiStatus) {
            wiFiStatus = status;
            checkWiFiStatus();
        }
    }

    void WiFiManager::initWiFi() {
        IO::Logger::info("|  - Connecting to WI-FI .");
        // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
        WiFi.mode(WIFI_STA);
        delay(1000); // TODO: is this delay necessary?
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
    }

    bool WiFiManager::checkWiFiStatus() {
        bool wpsSuccess = false;
        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED) {
            digitalWrite(Config::StatusLedPin, LOW);
            IO::Logger::info("|  ✔ Connected to '%s'", WiFi.SSID().c_str());
            IO::Logger::info("|  ✔ IP: %s", WiFi.localIP().toString().c_str());
            wpsSuccess = true;
        } else if (status == WL_CONNECTION_LOST || status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED) {
            digitalWrite(Config::StatusLedPin, HIGH);
            IO::Logger::error("|  x Lost connection to WiFi");
            initWiFi();
        } else {
            digitalWrite(Config::StatusLedPin, HIGH);
            IO::Logger::error("|  x Not connected to WiFi (state='%d')", status);
        }
        return wpsSuccess;
    }

    bool WiFiManager::startWPS() {
        digitalWrite(Config::StatusLedPin, LOW);
        WiFi.disconnect();
        delay(100);
        WiFi.disconnect();
        delay(400);
        digitalWrite(Config::StatusLedPin, HIGH);
        IO::Logger::info ("|  >> Press WPS button on your router <<");
        bool wpsSuccess = WiFi.beginWPSConfig();
        return wpsSuccess;
    }

}