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
 * Based on: https://github.com/xDWart/MQTTbroker
 *
 * Releases:
 * - SEE .h HEADER FILE
 *
 */

#include "MQTTBrokerMini.h"

namespace Net { namespace MQTT {

    static MQTTBrokerClient_t brokerClients[MQTTBROKER_CLIENT_MAX + 1]; //Last is MQTTBROKER_LOCAL_CLIENT_ID

    MQTTBrokerMini::MQTTBrokerMini(WebSocketsServer *webSocket) {
        WS = webSocket;
    }

    void MQTTBrokerMini::setCallback(const callback_t& cb) {
        callback = cb;
    }

    void MQTTBrokerMini::runCallback(uint8_t num, Events_t event, uint8_t *topic_name, uint16_t length_topic_name,
                                      uint8_t *payload, uint16_t length_payload) {
        if (callback) {
            callback(num, &event, topic_name, length_topic_name, payload, length_payload);
        }
    }

    void MQTTBrokerMini::begin() {
        setCallback(nullptr);
        for (auto &MQTTclient : brokerClients) {
            MQTTclient.status = false;
        }
    }

    void MQTTBrokerMini::parsing(uint8_t num, uint8_t *payload, uint16_t length) {
        if (numIsIncorrect(num)) return;

        switch (*payload >> 4) {
            case CONNECT: {
                    // 1 - CONNECT

                    uint8_t variable_header[2];
                    uint8_t Protocol_level = payload[8];
                    //uint8_t Connect_flags = payload[9];
                    uint16_t Length_MSB_LSB = MSB_LSB(&payload[12]);

                    // Create a new session anyway
                    variable_header[0] = 0x01 & SESSION_PRESENT_ZERO;

                    if (Protocol_level == MQTT_VERSION_3_1_1) {
                        variable_header[1] = CONNECT_ACCEPTED;
                        connect(num);
                        sendAnswer(num, CONNACK, 0, 2, variable_header, 2);
                        runCallback(num, EVENT_CONNECT, &payload[14], Length_MSB_LSB);
                    } else {
                        variable_header[1] = CONNECT_REFUSED_UPV;
                        sendAnswer(num, CONNACK, 0, 2, variable_header, 2);
                        disconnect(num);
                    }
                } break;
            case PUBLISH: {
                    // 3 - PUBLISH
                    uint8_t DUP = (*payload >> 3) & 0x1;
                    uint8_t QoS = (*payload >> 1) & 0x3;
                    uint8_t RETAIN = (*payload) & 0x1;
                    //uint8_t Remaining_length = payload[1];
                    uint16_t Remaining_length = 0;
                    //uint16_t Length_topic_name = MSB_LSB(&payload[2]);
                    uint16_t Length_topic_name = 0;
#define MAX_NO_OF_REMAINING_LENGTH_BYTES 4
                    uint16_t len = 0;
                    uint8_t multiplier = 1;
                    uint8_t offset = -1;
                    do {
                        if (++len > MAX_NO_OF_REMAINING_LENGTH_BYTES) {
                            // TODO: HANDLE MALFORMED PACKET
                            break;
                        }
                        Remaining_length += ((payload[len] & 127) * multiplier);
                        multiplier *= 128;
                        offset++;
                    } while ((payload[len] & 128) != 0);

                    //Length_topic_name = MSB_LSB(&payload[len]);
                    Length_topic_name = (payload[len + 1] * 256) + payload[len + 2];

                    uint8_t *Packet_identifier = nullptr;
                    uint8_t Packet_identifier_length = 0;

                    if (QoS > 0) {
                        Packet_identifier = &payload[4 + Length_topic_name];
                        Packet_identifier_length = 2;
                    } // else without packet identifier

                    runCallback(num, EVENT_PUBLISH, &payload[4 + offset], Length_topic_name,
                                &payload[4 + offset + Packet_identifier_length + Length_topic_name],
                                Remaining_length - 2 - Packet_identifier_length - Length_topic_name);

                    //QoS = 0 does not require to answer
                } break;
            case SUBSCRIBE: {
                    // 8 - SUBSCRIBE
                    //uint16_t Packet_identifier = MSB_LSB(&payload[2]);
                    uint16_t Length_MSB_LSB = MSB_LSB(&payload[4]);
                    uint8_t requestedQoS = payload[6 + Length_MSB_LSB];
                    sendAnswer(num, SUBACK, 0, 3, &payload[2], 2, &requestedQoS, 1);
                    runCallback(num, EVENT_SUBSCRIBE, &payload[6], Length_MSB_LSB);
                } break;
            case UNSUBSCRIBE: {
                    // 10 - UNSUBSCRIBE
                    //uint16_t Packet_identifier = MSB_LSB(&payload[2]);
                    //uint16_t Length_MSB_LSB = MSB_LSB(&payload[4]);
                    sendAnswer(num, UNSUBACK, 0, 2, &payload[2], 2);
                } break;
            case PINGREQ: {
                    // 12 - PINGREQ
                    sendAnswer(num, PINGRESP);
                } break;
            case DISCONNECT: {
                    // 14 - DISCONNECT
                    disconnect(num);
                } break;
            default: {
                    // UNKNOWN COMMAND
                    //DEBUG_MQTTBROKER(":%s [%d] >> UNKNOWN COMMAND\n", MQTTBROKER_NS_PREFIX, num);
                    //DEBUG_MQTTBROKER_HEX(payload, length);
            }
        }
    }

    void MQTTBrokerMini::publish(uint8_t num, uint8_t* topic, uint16_t topic_length, uint8_t *payload, uint16_t length) {
        sendMessage(num, topic, topic_length, payload, length);
    }

    void MQTTBrokerMini::disconnect(uint8_t num) {
        if (numIsIncorrect(num)) return;
        brokerClients[num].status = false;
        WS->disconnect(num);
        runCallback(num, EVENT_DISCONNECT);
    }

    bool MQTTBrokerMini::clientIsConnected(uint8_t num) {
        if (num == MQTTBROKER_LOCAL_CLIENT_ID) return true; //Always true
        if (numIsIncorrect(num)) return false;
        return brokerClients[num].status;
    }

/*
* Private
*/

    String MQTTBrokerMini::data_to_string(uint8_t *data, uint16_t length) {
        String str;
        str.reserve(length);
        for (uint16_t i = 0; i < length; i++)
            str += (char)data[i];
        return str;
    }

    bool MQTTBrokerMini::numIsIncorrect(uint8_t num) {
        if (num >= MQTTBROKER_CLIENT_MAX) {
            return true;
        } else {
            return false;
        }
    }

    void MQTTBrokerMini::sendMessage(uint8_t num, uint8_t *topic_name, uint16_t length_topic_name, uint8_t *payload,
                                      uint16_t length_payload) {
        if (!clientIsConnected(num)) return;

        uint32_t i;
        uint32_t remaining_length = length_topic_name + length_payload;

        remaining_length += 2; // protocol name length is stored in 2 bytes

        // bytes required to store remaining length number
        int rc = 0;
        if (remaining_length < 128) {
            rc = 1;
        } else if (remaining_length < 16384) {
            rc = 2;
        } else if (remaining_length < 2097152) {
            rc = 3;
        } else {
            rc = 4;
        }

        const uint32_t buffer_size = remaining_length + rc + MQTTBROKER_VHEADER_MIN_LENGTH;

#ifdef BOARD_HAS_PSRAM
        auto answer_msg = (uint8_t *) ps_malloc(buffer_size);
#else
        auto answer_msg = (uint8_t *) malloc(buffer_size);
#endif

        if (answer_msg == nullptr) {
            Serial.println("Error: could not allocate message buffer.");
            return;
        }

        answer_msg[0] = (PUBLISH << 4) | 0x00; //DUP, QoS, RETAIN

        int cb = 1;
        // remaining length
        uint16_t length = remaining_length;
        do {
            char d = (length % 128);
            length /= 128;
            /* if there are more digits to encode, set the top bit of this digit */
            if (length > 0) {
                d |= 0x80;
            }
            answer_msg[cb++] = (uint8_t)d;
        } while (length > 0);
        // topic length
        answer_msg[cb++] = length_topic_name >> 8;
        answer_msg[cb] = length_topic_name & 0xFF;

        rc += MQTTBROKER_VHEADER_MIN_LENGTH;

        // topic name
        for (i = 0; i < length_topic_name; i++){
            answer_msg[rc + i] = *(topic_name++);
        }
        // message payload
        for (i = 0; i < length_payload; i++) {
            answer_msg[length_topic_name + rc + i] = *(payload++);
        }

        int data_length = remaining_length + rc;
        WS->sendBIN(num, answer_msg, data_length - 2);

        free(answer_msg);
    }

    void MQTTBrokerMini::connect(uint8_t num) {
        brokerClients[num].status = true;
    }

    uint16_t MQTTBrokerMini::MSB_LSB(uint8_t *msb_byte) {
        return (uint16_t) (*msb_byte << 8) + *(msb_byte + 1);
    }

    void MQTTBrokerMini::sendAnswer(uint8_t num, uint8_t fixed_header_comm, uint8_t fixed_header_lsb,
                                     uint8_t fixed_header_remaining_length, uint8_t *variable_header,
                                     uint8_t variable_header_length, uint8_t *payload, uint16_t payload_length) {
        uint8_t i;
        uint8_t answer_msg[fixed_header_remaining_length + 2];

        answer_msg[0] = (fixed_header_comm << 4) | fixed_header_lsb;
        answer_msg[1] = fixed_header_remaining_length;
        for (i = 2; i < variable_header_length + 2; i++) answer_msg[i] = *(variable_header++);

        switch (fixed_header_comm) {
            case CONNACK: //2
                //DEBUG_MQTTBROKER(":%s [%d] << CONNACK\n", MQTTBROKER_NS_PREFIX, num);
                break;
            case PUBACK: //4 QoS level 1
                //DEBUG_MQTTBROKER(":%s [%d] << PUBACK\n", MQTTBROKER_NS_PREFIX, num);
                break;
            case PUBREC: //5 QoS level 2, part 1
                break;
            case PUBREL: //6 QoS level 2, part 2
                break;
            case PUBCOMP: //7 QoS level 2, part 3
                break;
            case SUBACK: //9
                answer_msg[i] = *payload;
                //DEBUG_MQTTBROKER(":%s [%d] << SUBACK\n", MQTTBROKER_NS_PREFIX, num);
                break;
            case UNSUBACK: //11
                //DEBUG_MQTTBROKER(":%s [%d] << UNSUBACK\n", MQTTBROKER_NS_PREFIX, num);
                break;
            case PINGRESP: //13
                //DEBUG_MQTTBROKER(":%s [%d] << PINGRESP\n", MQTTBROKER_NS_PREFIX, num);
                break;
            default:
                return;
        }
        //DEBUG_MQTTBROKER_HEX((uint8_t *) &answer_msg, fixed_header_remaining_length + 2);

        WS->sendBIN(num, (const uint8_t *) &answer_msg, fixed_header_remaining_length + 2);
    }

    MQTTBrokerClient_t* MQTTBrokerMini::getClients() {
        return brokerClients;
    }

    void MQTTBrokerMini::broadcast(uint8_t num, uint8_t* topic_name, uint16_t topic_length, uint8_t *payload, uint16_t length_payload) {
        for (uint8_t i = 0; i < MQTTBROKER_CLIENT_MAX; i++) {
            // TODO: send only if subscribed to topic
            // TODO; -> subscription not yet handled!
            if (i != num && brokerClients[i].status) {
                publish(i, topic_name, topic_length, payload, length_payload);
            }
        }
    }

}}
