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
 * - 2019-01-10 Initial release
 *
 */

#include "NetManager.h"

namespace Net {

    using namespace IO;

    static HTTPServer httpServer;

    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP);
    // Variables to save date and time
    String formattedDate;
    String dayStamp;
    String timeStamp;

    bool NetManager::begin() {

        Logger::info("+ Starting NetManager");

        // WI-FI will not boot without this delay!!!
        delay(2000);

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

        httpServer = new HTTPServer();
        httpServer->begin();

        mqttServer = new MQTTServer();
        mqttServer->begin();

        // Initialize a NTPClient to get time
        timeClient.begin();
        // Set offset time in seconds to adjust for your timezone, for example:
        // GMT +1 = 3600
        // GMT +8 = 28800
        // GMT -1 = -3600
        // GMT 0 = 0
        timeClient.setTimeOffset(0);

        return wpsSuccess;
    }

    void NetManager::loop() {
        Logger::verbose("%s loop() >> BEGIN", NETMANAGER_LOG_PREFIX);

        // TODO: this "while" seem the only way to make NTPClient work
        while (WiFi.isConnected() && !timeClient.update()) {
            timeClient.forceUpdate();
            // The formattedDate comes with the following format:
            // 2018-05-28T16:00:13Z
            // We need to extract date and time
            formattedDate = timeClient.getFormattedDate();
            Logger::info("NTP Time: %s", formattedDate.c_str());
        }

        Logger::verbose("%s loop() << END", NETMANAGER_LOG_PREFIX);
    }

    String NetManager::setStatus(int s) {
        String stat = R"({"status":")" + String(s) + "\"}";
        return stat;
    }

    HTTPServer NetManager::getHttpServer() {
        return *httpServer;
    }

    MQTTServer NetManager::getMQTTServer() {
        return *mqttServer;
    }

    NetManager::NetManager() {

    }
    NetManager::~NetManager() {
        // TODO: !!!! IMPLEMENT DESTRUCTOR AS WELL FOR HttpServer and MQTTServer classes
        delete httpServer;
        delete mqttServer;
    }

    NTPClient NetManager::getTimeClient() {
        return timeClient;
    }

}