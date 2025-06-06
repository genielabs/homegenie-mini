/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

    NetManager::NetManager() = default;
    NetManager::~NetManager() {
        // TODO: !!!! IMPLEMENT DESTRUCTOR AS WELL FOR HttpServer and MQTTServer classes
        delete httpServer;
        delete webSocket;
#ifndef DISABLE_MQTT_CLIENT
        delete mqttClient;
#endif
#ifndef DISABLE_MQTT_BROKER
        delete mqttServer;
#endif
    }

    void NetManager::begin() {

        Logger::info("+ Starting NetManager");
#ifndef DISABLE_BLUETOOTH
        bluetoothManager = new BluetoothManager();
        bluetoothManager->begin();
#endif
        wiFiManager = new WiFiManager();

        httpServer = new HTTPServer();
        httpServer->addHandler(this);
        httpServer->begin();

        // start the websocket server
        webSocket = new WebSocketsServer(8188, "", "hg");
        webSocket->onEvent([this](uint8_t num, WStype_t type, uint8_t * payload, size_t length){
            switch (type) {
                case WStype_DISCONNECTED:
                    // client disconnected
                    Serial.printf("[%u] Disconnected!\n", num);
                    break;
                case WStype_CONNECTED: {
                        // new client connected
                        IPAddress ip; // = webSocket->remoteIP(num);
                        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                    }
                    break;
                case WStype_TEXT: {
                        // clear-text message received
//                        Serial.printf("[%u] TEXT\t%s\n", num, payload);
                        char message[length + 5];
                        sprintf(message, "api/%s", payload);
                        auto cb = WebSocketResponseCallback(webSocket, num, nullptr);
                        netRequestHandler->onNetRequest(webSocket, message, &cb);
                    }
                    break;
                case WStype_ERROR:
                    break;
                case WStype_BIN: {
                        // binary-packed message received
                        MsgPack::Unpacker unpacker;
                        std::array<String, 2> req;
                        unpacker.feed(payload, length);
                        unpacker.unpack(req);
                        unpacker.clear();
                        // route message with response callback
                        String rid = req[0];
                        String request = "api/" + req[1];
//                        Serial.printf("[%u] BIN\t%s\t%s\n", num, rid.c_str(), request.c_str());
                        auto cb = WebSocketResponseCallback(webSocket, num, &rid);
                        netRequestHandler->onNetRequest(webSocket, request.c_str(), &cb);
                    }
                    break;
                case WStype_FRAGMENT_TEXT_START:
                    break;
                case WStype_FRAGMENT_BIN_START:
                    break;
                case WStype_FRAGMENT:
                    break;
                case WStype_FRAGMENT_FIN:
                    break;
                case WStype_PING:
                    break;
                case WStype_PONG:
                    break;
            }
        });
        webSocket->begin();
        Logger::info("|  ✔ WebSocket server");

#ifndef DISABLE_MQTT_BROKER
        mqttServer = new MQTTServer();
        mqttServer->onRequest([this](uint8_t num, const char* cid, const char* tid, const char* domain, const char* address, const char* command, const char* options, const char* data) {
            auto c = String(command);
            if (c == "Module.Describe") {
                String topic = Config::system.id;
                topic.concat("/");
                topic.concat(domain);
                topic.concat("/");
                topic.concat(address);
                topic.concat("/description");
                String apiCommand = "/api/";
                apiCommand.concat(IOEventDomains::HomeAutomation_HomeGenie);
                apiCommand.concat("/Config/Modules.Get/");
                apiCommand.concat(domain);
                apiCommand.concat("/");
                apiCommand.concat(address);
                auto cb = MQTTResponseCallback((MQTTChannel*)mqttServer, &topic);
                netRequestHandler->onNetRequest(mqttServer, apiCommand.c_str(), &cb);
            } else {
                String topic = cid;
                topic.concat("/MQTT.Listeners/");
                topic.concat(tid);
                topic.concat("/response");
                String apiCommand = String("/api/");
                apiCommand.concat(domain);
                apiCommand.concat("/");
                apiCommand.concat(address);
                apiCommand.concat("/");
                apiCommand.concat(command);
                apiCommand.concat("/");
                apiCommand.concat(options);
                apiCommand.concat("/plain/");
                apiCommand.concat(data);
                auto cb = MQTTResponseCallback((MQTTChannel*)mqttServer, &topic);
                netRequestHandler->onNetRequest(mqttServer, apiCommand.c_str(), &cb);
            }

        });
        mqttServer->begin();
        Logger::info("|  ✔ MQTT broker");
#endif

#ifndef DISABLE_MQTT_CLIENT
        mqttClient = new MQTTClient();
        Net::MQTTClient::requestHandler = this;
        Logger::info("|  ✔ MQTT client");
#endif

        timeClient = new TimeClient();
        timeClient->begin();
    }


    void NetManager::loop() {
        Logger::verbose("%s loop() >> BEGIN", NETMANAGER_LOG_PREFIX);

        webSocket->loop();

        Logger::verbose("%s loop() << END", NETMANAGER_LOG_PREFIX);
    }

    TimeClient* NetManager::getTimeClient() {
        return timeClient;
    }

#ifndef DISABLE_BLUETOOTH
    BluetoothManager& NetManager::getBLEManager() {
        return *bluetoothManager;
    }
#endif

    WiFiManager& NetManager::getWiFiManager() {
        return *wiFiManager;
    }

    HTTPServer& NetManager::getHttpServer() {
        return *httpServer;
    }

#ifndef DISABLE_MQTT_BROKER
    MQTTServer& NetManager::getMQTTServer() {
        return *mqttServer;
    }
#endif
#ifndef DISABLE_MQTT_CLIENT
    MQTTClient& NetManager::getMQTTClient() {
        return *mqttClient;
    }

    bool NetManager::onMqttRequest(void *sender, String &req, String &data, String &tid) {
        String topic = Config::system.id + "/MQTT.Listeners/" + tid + "/response";
        auto callback = MQTTResponseCallback((MQTTChannel*)mqttClient, &topic);
        netRequestHandler->onNetRequest(sender, req.c_str(), &callback);
        return false;
    }
#endif

    WebSocketsServer& NetManager::getWebSocketServer() {
        return *webSocket;
    }

    void NetManager::setRequestHandler(NetRequestHandler* handler) {
        netRequestHandler = handler;
    }
}
