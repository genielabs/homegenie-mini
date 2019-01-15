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
 * - 2019-14-01 Initial release
 *
 */

#include "MQTTServer.h"

namespace Net {

    static WebSocketsServer *ws;
    static MQTTBrokerMini *mb;

    void MqttServer::begin() {

        // TODO: add <MQTTBrokerMiniConfig> class to store configuration
        webSocket = new WebSocketsServer(8000, "", "mqtt");
        mqttBroker = new MQTTBrokerMini(webSocket);

        ws = webSocket;
        mb = mqttBroker;

        webSocket->begin();
        webSocket->onEvent(webSocketEventStatic);

        mqttBroker->begin();
        mqttBroker->setCallback(mqttCallbackStatic);

    }

    void MqttServer::loop() {
        webSocket->loop();

        static int cnt = 1;
        // Just for testing, sends a message
        if (cnt > 100000) {
            String prefix = "/homegenie";
            String deviceID = "01";
            int num = 0;

            String test = R"({ "Message": "Hello World!" })";

            // TODO: implment MQTTBBroker::broadcast( .. ) for sending to all clients

            // send to client with id = `num`
            mqttBroker->publish(num, (prefix).c_str(), (uint8_t *) test.c_str(), test.length());
            cnt = 0;
        }
        cnt++;

    }

    void MqttServer::mqttCallbackStatic(uint8_t num, Events_t event, String topic_name, uint8_t *payload,
                                        uint16_t length_payload) {
        auto msg = String((char*)payload);
        switch (event){
            case EVENT_CONNECT:
                Serial.printf("[%d] New connect from ", num);  Serial.println(topic_name);
                break;
            case EVENT_SUBSCRIBE:
                Serial.printf("[%d] Subscribe to ", num);  Serial.println(topic_name);
                break;
            case EVENT_PUBLISH:
                Serial.printf("[%d] Receive publish to '%s'\n", num, topic_name.c_str());
                // broadcast message to subscribed clients
                for (uint8_t i = 0; i < MQTTBROKER_CLIENT_MAX; i++) {
                    // TODO: send only if subscribed to topic
                    if (i != num && mb->getClients()[i].status) {
                        mb->publish(i, (topic_name).c_str(), payload, length_payload);
                    }
                }
                // TODO: ... IMPLEMENT HG API HANDLE TOPIC
                if (topic_name == "TODO_CHANGE_WITH_MY_ID/control") {
                    // TODO: ....
                }
                break;
            case EVENT_DISCONNECT:
                Serial.printf("[%d] Disconnect\n\n", num);
                break;
        }
    }

    void MqttServer::webSocketEventStatic(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        switch(type) {
            case WStype_DISCONNECTED:
                if (mb->clientIsConnected(num)) mb->disconnect(num);
                break;
            case WStype_BIN:
                mb->parsing(num, payload, length);
                break;
        }
    }

}
