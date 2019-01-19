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

    void MQTTServer::begin() {

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

    void MQTTServer::loop() {
        webSocket->loop();
        // Just for testing, sends a message periodically
        static int cnt = 1;
        if (cnt > 100000) {
            String prefix = "/homegenie";
            String deviceID = "world";
            String test = R"({ "Message": "Hello from HomeGenie-Mini!" })";
            mb->broadcast((prefix).c_str(), (uint8_t *)test.c_str(), (uint16_t)test.length());
            cnt = 0;
        }
        cnt++;
    }

    void MQTTServer::mqttCallbackStatic(uint8_t num, Events_t event, String topic_name, uint8_t *payload,
                                        uint16_t length_payload) {
        auto msg = String((char*)payload);
        switch (event){
            case EVENT_CONNECT:
                IO::Logger::trace("%s [%d] >> CONNECT from '%s'", MQTTBROKER_LOG_PREFIX, num, topic_name.c_str());
                break;
            case EVENT_SUBSCRIBE:
                IO::Logger::trace("%s [%d] >> SUBSCRIBE to '%s'", MQTTBROKER_LOG_PREFIX, num, topic_name.c_str());
                break;
            case EVENT_PUBLISH:
                IO::Logger::trace("%s [%d] >> PUBLISH to '%s'", MQTTBROKER_LOG_PREFIX, num, topic_name.c_str());
                // TODO: ... IMPLEMENT HG API HANDLE TOPIC
                if (topic_name == "TODO_CHANGE_WITH_MY_ID/control") {
                    // TODO: Control API
                } else {
                    // broadcast message  to subscribed clients
                    mb->broadcast(num, (topic_name).c_str(), payload, length_payload);
                }
                break;
            case EVENT_DISCONNECT:
               IO::Logger::trace("%s [%d] >> DISCONNECT =/", MQTTBROKER_LOG_PREFIX, num);
                break;
        }
    }

    void MQTTServer::webSocketEventStatic(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        switch(type) {
            case WStype_DISCONNECTED:
                if (mb->clientIsConnected(num)) mb->disconnect(num);
                break;
            case WStype_BIN:
                mb->parsing(num, payload, (uint16_t)length);
                break;
        }
    }

    void MQTTServer::broadcast(String *topic, String *payload) {
        mb->broadcast(*topic, (uint8_t *)payload->c_str(), (uint16_t)payload->length());
    }

}
