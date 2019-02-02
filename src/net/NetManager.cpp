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

#include <service/HomeGenie.h>
#include "NetManager.h"

namespace Net {

    using namespace IO;

    HTTPServer httpServer;

    // Time sync
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

        bool wpsSuccess;
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

        // start the websocket server
        webSocket = new WebSocketsServer(88, "", "hg");
        webSocket->onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t lenght){
            switch (type) {
                case WStype_DISCONNECTED:             // if the websocket is disconnected
                    Serial.printf("[%u] Disconnected!\n", num);
                    break;
                case WStype_CONNECTED: {              // if a new websocket connection is established
                    IPAddress ip; // = webSocket->remoteIP(num);
                    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                    //webSocket.sendTXT(num, "... ping ...");
                }
                    break;
                case WStype_TEXT:                     // if new text data is received
                    Serial.printf("[%u] get Text: %s\n", num, payload);
                    break;
            }
        });
        webSocket->begin();
        Logger::info("|  ✔ WebSocket server");

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

        webSocket->loop();

        if (WiFi.isConnected() && !timeClient.update()) {
            timeClient.forceUpdate();
            // The formattedDate comes with the following format:
            // 2018-05-28T16:00:13Z
            // We need to extract date and time
            formattedDate = timeClient.getFormattedDate();
            Logger::info("NTP Time: %s", formattedDate.c_str());
        }

        Logger::verbose("%s loop() << END", NETMANAGER_LOG_PREFIX);
    }

    HTTPServer* NetManager::getHttpServer() {
        return httpServer;
    }

    MQTTServer* NetManager::getMQTTServer() {
        return mqttServer;
    }

    WebSocketsServer* NetManager::getWebSocketServer() {
        return webSocket;
    }

    NetManager::NetManager() {

    }
    NetManager::~NetManager() {
        // TODO: !!!! IMPLEMENT DESTRUCTOR AS WELL FOR HttpServer and MQTTServer classes
        delete httpServer;
        delete mqttServer;
        delete webSocket;
    }

    NTPClient NetManager::getTimeClient() {
        return timeClient;
    }

}