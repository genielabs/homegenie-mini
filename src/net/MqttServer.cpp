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

#include "MqttServer.h"

namespace Net {

    static WebSocketsServer *ws;
    static MQTTbroker_lite *mb;

    void MqttServer::begin() {

        webSocket = new WebSocketsServer(8000, "", "mqtt");
        mqttBroker = new MQTTbroker_lite(webSocket);

        ws = webSocket;
        mb = mqttBroker;

        webSocket->begin();
        webSocket->onEvent(webSocketEventStatic);
        //(*WebSocketServerEvent)(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
        mqttBroker->begin();
        mqttBroker->setCallback(mqttCallbackStatic);

    }

    int cnt = 1;
    void MqttServer::loop() {

        webSocket->loop();

        if (cnt > 100000) {
            String prefix = "/homegenie";
            String deviceID = "01";
            int num = 0;

            String test = R"({ "Message": "Hello World!" })";

            // TODO: implment sending to all clients

//            mqttBroker->publish((prefix + "/" + deviceID + "/config").c_str(), (uint8_t *) test.c_str(),
//                               test.length());
            mqttBroker->publish(num, (prefix).c_str(), (uint8_t *) test.c_str(),
                                test.length());
            cnt = 0;
        }
        cnt++;

    }


    /*
    void NetManager::mqttCallbackStatic(uint8_t num, Events_t event, String topic_name, uint8_t *payload,
                                        uint8_t length_payload) {
// TODO: MANAGE connected client list
        Serial.printf("Receive publish to "); Serial.print(topic_name + " ");
        auto msg = String((char*)payload);
        Serial.print(msg);
        if (topic_name == "/homegenie/control") {
            Serial.print(msg);
        }
        Serial.println();

        //if (topic_name == prefix) pubConfig();
    }
*/


    void MqttServer::mqttCallbackStatic(uint8_t num, Events_t event, String topic_name, uint8_t *payload,
                                        uint8_t length_payload) {
// TODO: MANAGE connected client list
        auto msg = String((char*)payload);
        switch (event){
            case EVENT_CONNECT:
//                digitalWrite(BLUE_ESP_LED, LED_ON);
                Serial.printf("[%d] New connect from ", num);  Serial.println(topic_name);
                break;
            case EVENT_SUBSCRIBE:
                Serial.printf("[%d] Subscribe to ", num);  Serial.println(topic_name);
                break;
            case EVENT_PUBLISH:
                Serial.printf("[%d] Receive publish to ", num); Serial.print(topic_name + " ");
                Serial.print(msg);
                mb->publish(1, (topic_name).c_str(), (uint8_t *) msg.c_str(),
                            msg.length());
                // TODO: ... HANDLE TOPIC
                if (topic_name == "TODO_CHANGE_TOPIC/control") {
//                    if (*payload == '0') digitalWrite(BLUE_NODE_LED, LED_OFF);
//                    else digitalWrite(BLUE_NODE_LED, LED_ON);
                }
                Serial.println();
//TODO:                if (topic_name == prefix) pubConfig(num);
                break;
            case EVENT_DISCONNECT:
//                digitalWrite(BLUE_ESP_LED, LED_OFF);
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
