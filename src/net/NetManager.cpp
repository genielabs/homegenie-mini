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
 * - 2019-10-01 Initial release
 *
 */

#include "NetManager.h"

namespace Net {

    using namespace IO;

    NetManager::NetManager() {

    }

    bool NetManager::begin() {
        Logger::infoN("|  - Connecting to WI-FI .");
        // WPS works in STA (Station mode) only -> not working in WIFI_AP_STA !!!
        WiFi.mode(WIFI_STA);
        delay(1000); // TODO: is this delay necessary?
        // WiFi.begin("foobar",""); // make a failed connection
        WiFi.begin(WiFi.SSID().c_str(), WiFi.psk().c_str());
        while (WiFi.status() == WL_DISCONNECTED) {
            delay(2000);
            Serial.print(".");
        }
        Serial.println();

        bool wpsSuccess = false;
        wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED) {
            Logger::info("|  ✔ Connected to '%s'", WiFi.SSID().c_str());
            wpsSuccess = true;
        } else {
            Logger::error("|  ! Not connected to WiFi (state='%d')", status);
            Logger::info ("|  >> Press WPS button on your router <<");
            //while(!Serial.available()) { ; }
            //if(!startWPSPBC()) {
            //    Serial.println("Failed to connect with WPS :-(");
            //}
            wpsSuccess = WiFi.beginWPSConfig();
            if (wpsSuccess) {
                // Well this means not always success :-/ in case of a timeout we have an empty ssid
                String newSSID = WiFi.SSID();
                if (newSSID.length() > 0) {
                    // WPSConfig has already connected in STA mode successfully to the new station.
                    Logger::info("|  ✔ Successfully connected to '%s'", newSSID.c_str());
                    // save to config and use next time or just use - WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str());
                    //qConfig.wifiSSID = newSSID;
                    //qConfig.wifiPWD = WiFi.psk();
                    //saveConfig();
                } else {
                    wpsSuccess = false;
                }
            }

        }

        if (wpsSuccess) {
            Logger::info("|  ✔ IP: %s", WiFi.localIP().toString().c_str());
        }

        return wpsSuccess;
    }

}