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
 * - 2019-01-10 Initial release
 *
 */

#include "NetManager.h"

namespace Net {

    using namespace IO;

    // Time sync
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP);
#ifdef ESP32
    ESP32Time rtc(0);
#endif
    // Variables to save date and time
    String formattedDate;
    String dayStamp;
    String timeStamp;
#ifdef ESP32
    bool rtcTimeSet = (esp_reset_reason() != ESP_RST_POWERON && esp_reset_reason() != ESP_RST_UNKNOWN);
#else
    bool rtcTimeSet = false;
#endif
    long lastTimeCheck = -100000;
#ifndef CONFIGURE_WITH_WPA
    BluetoothSerial SerialBT;
#endif

    NetManager::NetManager() {
        // TODO: ...
    }
    NetManager::~NetManager() {
        // TODO: !!!! IMPLEMENT DESTRUCTOR AS WELL FOR HttpServer and MQTTServer classes
        delete httpServer;
        delete mqttServer;
        delete webSocket;
    }

    bool NetManager::begin() {

        Logger::info("+ Starting NetManager");
#ifndef DISABLE_BLE
        bleManager = new BLEManager();
#endif
        wiFiManager = new WiFiManager();
        bool wpsSuccess = wiFiManager->checkWiFiStatus();

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


#ifndef CONFIGURE_WITH_WPA
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, true);
        String ssid = preferences.getString("wifi:ssid");
        String pass = preferences.getString("wifi:password");
        // start SerialBT only if Wi-Fi is not configured
        if (ssid.isEmpty() || pass.isEmpty()) {
            SerialBT.begin(CONFIG_BUILTIN_MODULE_NAME);
        }
        preferences.end();
#endif



        return wpsSuccess;
    }

    void NetManager::loop() {
        Logger::verbose("%s loop() >> BEGIN", NETMANAGER_LOG_PREFIX);

#ifndef CONFIGURE_WITH_WPA

        // CONFIGURE WITH BLUETOOTH
        if (SerialBT.available()) {
            String message = SerialBT.readStringUntil('\n');
            if (message != nullptr) {

                Serial.println(message);

                // ECHO? --> SerialBT.println(message);

                Preferences preferences;
                preferences.begin(CONFIG_SYSTEM_NAME, false);

                if (message.startsWith("#CONFIG:device-name ")) {
                    preferences.putString("device:name", message.substring(20));
                }
                if (message.startsWith("#CONFIG:wifi-ssid ")) {
                    preferences.putString("wifi:ssid", message.substring(18));
                }
                if (message.startsWith("#CONFIG:wifi-password ")) {
                    preferences.putString("wifi:password", message.substring(22));
                }
                if (message.startsWith("#CONFIG:system-time ")) {
                    String time = message.substring(20);
                    long seconds = time.substring(0, time.length() - 3).toInt();
                    long ms = time.substring(time.length() - 3).toInt();
                    rtc.setTime(seconds, ms);
                }

                preferences.end();

                if (message.equals("#RESET")) {
                    SerialBT.disconnect();
                    SerialBT.end();
                    IO::Logger::info("RESET!");
                    delay(2000);
                    esp_restart();
                }

            }
        }

#endif

        if (ESP_WIFI_STATUS == WL_CONNECTED) {
            webSocket->loop();
        }

        if (rtcTimeSet) {
#ifdef ESP32
            if (millis() - lastTimeCheck > 60000) {
                lastTimeCheck = millis();
                if (!timeClient.isUpdated()) {
                    // sync TimeClient with RTC
                    timeClient.setEpochTime(rtc.getLocalEpoch());
                    Logger::info("|  - TimeClient: synced with RTC");
                }
            }
#endif
        } else if (WiFi.isConnected() && millis() - lastTimeCheck > 60000) {
            lastTimeCheck = millis();
            if (!timeClient.isUpdated()) {
                if (timeClient.update()) {
                    // TimeClient synced with NTP
#ifdef ESP32
                    rtc.setTime(timeClient.getEpochTime(), 0);
                    rtcTimeSet = true;
                    Logger::info("|  - RTC updated via TimeClient (NTP)");
#endif
                } else {
                    // NTP Update failed
                    digitalWrite(Config::StatusLedPin, HIGH);
                    Logger::warn("|  x TimeClient: NTP update failed!");
                }
            }
        }

        Logger::verbose("%s loop() << END", NETMANAGER_LOG_PREFIX);
    }

#ifndef DISABLE_BLE
    BLEManager& NetManager::getBLEManager() {
        return *bleManager;
    }
#endif

    WiFiManager& NetManager::getWiFiManager() {
        return *wiFiManager;
    }

    HTTPServer& NetManager::getHttpServer() {
        return *httpServer;
    }

    MQTTServer& NetManager::getMQTTServer() {
        return *mqttServer;
    }

    WebSocketsServer& NetManager::getWebSocketServer() {
        return *webSocket;
    }

    NTPClient& NetManager::getTimeClient() {
        return timeClient;
    }

}
