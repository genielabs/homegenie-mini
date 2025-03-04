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
 * - 2019-01-14 Initial release
 *
 */

#include "MQTTServer.h"

namespace Net {

    uint8_t* buf = nullptr;
    size_t totalLength = 0;

    void MQTTServer::begin() {

        // TODO: add <MQTTBrokerMiniConfig> class to store configuration
        webSocket = new WebSocketsServer(8000, "", "mqtt");
        mqttBroker = new MQTTBrokerMini(webSocket);

        webSocket->begin();
        webSocket->onEvent([this](uint8_t n, WStype_t t, uint8_t * p, size_t l) {
            webSocketEvent(n, t, p, l);
        });

        mqttBroker->begin();
        mqttBroker->setCallback([this](uint8_t n, const Net::MQTT::Events_t* e, uint8_t* topic_name, uint16_t topic_length, uint8_t* payload, uint16_t payload_length) {
            mqttCallback(n, e, topic_name, topic_length, payload, payload_length);
        });
        Logger::info("|  ✔ MQTT service");

    }

    void MQTTServer::loop() {
        if (ESP_WIFI_STATUS == WL_CONNECTED) {
            webSocket->loop();
        }
    }

    void MQTTServer::mqttCallback(uint8_t num, const Events_t* event, uint8_t* topic_name, uint16_t topic_length, uint8_t* payload,
                                        uint16_t length_payload) {
        switch (*event){
            case EVENT_CONNECT: {
                // IO::Logger::trace(":%s [%d] >> CONNECT from '%s'", MQTTBROKER_NS_PREFIX, num, (*topic_name).c_str());
                break;
            }
            case EVENT_SUBSCRIBE: {
                // IO::Logger::trace(":%s [%d] >> SUBSCRIBE to '%s'", MQTTBROKER_NS_PREFIX, num, (*topic_name).c_str());
                break;
            }
            case EVENT_PUBLISH: {
                // IO::Logger::trace(":%s [%d] >> PUBLISH to '%s'", MQTTBROKER_NS_PREFIX, num, (*topic_name).c_str());
                auto controlTopic = String ("/") + Config::system.id + String("/command");
#if ESP8266
                auto topic = mqttBroker->data_to_string(topic_name, topic_length);
#else
                auto topic = String(topic_name, topic_length);
#endif
                if (topic.endsWith(controlTopic)) { // initial part is the source node id, ending part is the destination node

                    JsonDocument doc;
                    deserializeJson(doc, payload, length_payload);
                    if (apiCallback != nullptr && doc.containsKey("Domain") && doc.containsKey("Address") && doc.containsKey("Command")) {
                        apiCallback(
                                num,
                                doc["ClientId"],
                                doc["TransactionId"],
                                doc["Domain"],
                                doc["Address"],
                                doc["Command"],
                                doc["OptionsString"],
                                doc["Data"]
                            );
                    }
                    doc.clear();

                } else {

                    // broadcast message to subscribed clients
                    mqttBroker->broadcast(topic_name, topic_length, payload, length_payload);

                }

                break;
            }
            case EVENT_DISCONNECT: {
                IO::Logger::trace(":%s [%d] >> DISCONNECT =/", MQTTBROKER_NS_PREFIX, num);
                break;
            }
        }
    }

    void MQTTServer::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        switch(type) {
            case WStype_DISCONNECTED: {
                if (MQTTBrokerMini::clientIsConnected(num)) mqttBroker->disconnect(num);
            }
            break;
            case WStype_TEXT: {
            }
            break;
            case WStype_FRAGMENT_BIN_START: {
                if (buf != nullptr) {
                    free(buf);
                }
                buf = (uint8_t*)malloc(sizeof(uint8_t) * length);
                if (buf != nullptr) {
                    memcpy(buf, payload, length);
                    totalLength = length;
                }
            }
                break;
            case WStype_FRAGMENT: {
                if (buf != nullptr) {
                    uint8_t* old = buf;
                    buf = (uint8_t*) malloc(sizeof(uint8_t) * (totalLength + length));
                    if (buf != nullptr) {
                        memcpy(buf, old, totalLength);
                        memcpy(&buf[totalLength], payload, length);
                        totalLength += length;
                    }
                    free(old);
                }
            }
                break;
            case WStype_FRAGMENT_FIN: {
                if (buf != nullptr) {
                    uint8_t* old = buf;
                    buf = (uint8_t*) malloc(sizeof(uint8_t) * (totalLength + length));
                    if (buf != nullptr) {
                        memcpy(buf, old, totalLength);
                        memcpy(&buf[totalLength], payload, length);
                        totalLength += length;
                    }
                    free(old);
                }
                if (buf != nullptr && totalLength > 0) {
                    mqttBroker->parsing(num, buf, (uint16_t) totalLength);
                }
                totalLength = 0;
                free(buf);
                buf = nullptr;
            }
                break;
            case WStype_BIN: {
                mqttBroker->parsing(num, payload, (uint16_t) length);
            }
            break;
        }
    }

    void MQTTServer::broadcast(uint8_t num, String *topic, String *payload) {
        mqttBroker->broadcast(num, (uint8_t*)topic->c_str(), topic->length(), (uint8_t *)payload->c_str(), (uint16_t)payload->length());
    }

    void MQTTServer::broadcast(String *topic, String *payload) {
        mqttBroker->broadcast((uint8_t*)topic->c_str(), topic->length(), (uint8_t*)payload->c_str(), payload->length());
    }

    void MQTTServer::broadcast(uint8_t* topic,uint16_t topic_length, uint8_t *payload, size_t length) {
        mqttBroker->broadcast(topic, topic_length, payload, length);
    }

}
