#include "MQTTbroker_lite.h"

#ifdef DEBUG_ESP_PORT
void PrintHex8_lite(uint8_t *data, uint8_t length)
{
  DEBUG_ESP_PORT.print("[");
  for (int i=0; i<length; i++) {
         if (data[i]<0x10) { DEBUG_ESP_PORT.print("0"); }
         DEBUG_ESP_PORT.print(data[i],HEX); DEBUG_ESP_PORT.print(" ");
  }
  DEBUG_ESP_PORT.print("]\n\n");
}
#define DEBUG_MQTTBROKER(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
#define DEBUG_MQTTBROKER_HEX(...) PrintHex8_lite( __VA_ARGS__ )
#else
#define DEBUG_MQTTBROKER(...) Serial.printf(__VA_ARGS__)
#define DEBUG_MQTTBROKER_HEX(...)
#endif

/*
 * Public
 */

MQTTbroker_lite::MQTTbroker_lite(WebSocketsServer *webSocket) {
    WS = webSocket;
}

void MQTTbroker_lite::setCallback(callback_t cb) {
    callback = cb;
}

void MQTTbroker_lite::unsetCallback(void) {
    callback = NULL;
}

void MQTTbroker_lite::runCallback(uint8_t num, Events_t event, uint8_t *topic_name, uint16_t length_topic_name,
                                  uint8_t *payload, uint16_t length_payload) {
    if (callback) {
        delay(0);
        callback(num, event, data_to_string(topic_name, length_topic_name), payload, length_payload);
    }
}

void MQTTbroker_lite::begin(void) {
    unsetCallback();
    for (uint8_t i = 0; i < MQTTBROKER_CLIENT_MAX + 1; i++) {
        MQTTclients[i].status = false;
    }
}

void MQTTbroker_lite::parsing(uint8_t num, uint8_t *payload, uint16_t length) {
    if (numIsIncorrect(num)) return;

    switch (*payload >> 4) {
        case CONNECT: //1
        {
            uint8_t variable_header[2];
            uint8_t Protocol_level = payload[8];
            uint8_t Connect_flags = payload[9];
            uint16_t Length_MSB_LSB = MSB_LSB(&payload[12]);

            variable_header[0] = 0x01 & SESSION_PRESENT_ZERO; //Anyway create a new Session

            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][RECEIVE] CONNECT [Protocol level = %d, Connect flags = %X]\n", num,
                             Protocol_level, Connect_flags);
            DEBUG_MQTTBROKER_HEX(payload, length);

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
        }
            break;
        case PUBLISH: //3
        {
            uint8_t DUP = (*payload >> 3) & 0x1;
            uint8_t QoS = (*payload >> 1) & 0x3;
            uint8_t RETAIN = (*payload) & 0x1;
            //uint8_t Remaining_length = payload[1];
            //uint16_t Length_topic_name = MSB_LSB(&payload[2]);
            uint16_t Remaining_length = 0;
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

            Serial.println();
            Serial.println(length);
            Serial.println(Remaining_length);
            Serial.println(Length_topic_name);
            Serial.println();

            for (uint16_t i = 0; i < length; i++) {
                Serial.print("   > ");Serial.println((byte)payload[i]);
            }


            uint8_t *Packet_identifier = NULL;
            uint8_t Packet_identifier_length = 0;

            if (QoS > 0) {
                Packet_identifier = &payload[4 + Length_topic_name];
                Packet_identifier_length = 2;
            } // else without packet identifier

            DEBUG_MQTTBROKER(
                    "[MQTT_BROKER][%d][RECEIVE] PUBLISH [DUP = %d, QoS = %d, RETAIN = %d, Rem_len = %d, Topic_len = %d]\n",
                    num, DUP, QoS, RETAIN, Remaining_length, Length_topic_name);
            DEBUG_MQTTBROKER_HEX(payload, length);

            runCallback(num, EVENT_PUBLISH, &payload[4 + offset], Length_topic_name,
                        &payload[4 + offset + Packet_identifier_length + Length_topic_name],
                        Remaining_length - 2 - Packet_identifier_length - Length_topic_name);

            //QoS = 0 don't need answer
        }
            break;
        case SUBSCRIBE: //8
        {
            uint16_t Packet_identifier = MSB_LSB(&payload[2]);
            uint16_t Length_MSB_LSB = MSB_LSB(&payload[4]);
            uint8_t Requesteed_QoS = payload[6 + Length_MSB_LSB];
            DEBUG_MQTTBROKER(
                    "[MQTT_BROKER][%d][RECEIVE] SUBSCRIBE [Packet identifier = %d, Length = %d, Requested QoS = %d]\n",
                    num, Packet_identifier, Length_MSB_LSB, Requesteed_QoS);
            DEBUG_MQTTBROKER_HEX(payload, length);
            sendAnswer(num, SUBACK, 0, 3, &payload[2], 2, &Requesteed_QoS, 1);
            runCallback(num, EVENT_SUBSCRIBE, &payload[6], Length_MSB_LSB);
        }
            break;
        case UNSUBSCRIBE: //10
        {
            uint16_t Packet_identifier = MSB_LSB(&payload[2]);
            uint16_t Length_MSB_LSB = MSB_LSB(&payload[4]);
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][RECEIVE] UNSUBSCRIBE [Packet identifier = %d, Length = %d]\n", num,
                             Packet_identifier, Length_MSB_LSB);
            DEBUG_MQTTBROKER_HEX(payload, length);
            sendAnswer(num, UNSUBACK, 0, 2, &payload[2], 2);
        }
            break;
        case PINGREQ: //12
        {
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][RECEIVE] PINGREQ\n", num);
            DEBUG_MQTTBROKER_HEX(payload, length);
            sendAnswer(num, PINGRESP);
        }
            break;
        case DISCONNECT: //14
        {
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][RECEIVE] DISCONNECT\n", num);
            DEBUG_MQTTBROKER_HEX(payload, length);
            disconnect(num);
        }
            break;
        default: {
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][RECEIVE] UNKNOWN COMMAND\n", num);
            DEBUG_MQTTBROKER_HEX(payload, length);
        }
    }
}

void MQTTbroker_lite::publish(uint8_t num, String topic, uint8_t *payload, uint16_t length) {
    sendMessage(num, (uint8_t *) &topic[0], topic.length(), payload, length);
}

void MQTTbroker_lite::disconnect(uint8_t num) {
    if (numIsIncorrect(num)) return;
    MQTTclients[num].status = false;
    WS->disconnect(num);
    runCallback(num, EVENT_DISCONNECT);
}

bool MQTTbroker_lite::clientIsConnected(uint8_t num) {
    if (num == MQTTBROKER_MY_NUM) return true; //Always true
    if (numIsIncorrect(num)) return false;
    return MQTTclients[num].status;
}

/*
 * Private
 */

String MQTTbroker_lite::data_to_string(uint8_t *data, uint16_t length) {
    String str;
    str.reserve(length);
    for (uint16_t i = 0; i < length; i++)
        str += (char) data[i];
    return str;
}

bool MQTTbroker_lite::numIsIncorrect(uint8_t num) {
    if (num >= MQTTBROKER_CLIENT_MAX) {
        return true;
    } else {
        return false;
    }
}

void MQTTbroker_lite::sendMessage(uint8_t num, uint8_t *topic_name, uint16_t length_topic_name, uint8_t *payload,
                                  uint16_t length_payload) {
    if (!clientIsConnected(num)) return;

    uint16_t i;
    uint16_t remaining_length = length_topic_name + length_payload;

    remaining_length += 2;

    int rc = 0;
    if (remaining_length < 128)
        rc = 1;
    else if (remaining_length < 16384)
        rc = 2;
//    else if (remaining_length < 2097152)
//        rc = 3;
//    else
//        rc = 2;

    uint8_t answer_msg[remaining_length + rc];

    answer_msg[0] = (PUBLISH << 4) | 0x00; //DUP, QoS, RETAIN

    Serial.printf("\nremaining length = %d\n\n", remaining_length);
    Serial.printf("\ntopic length = %d\n\n", length_topic_name);
    Serial.printf("\npayload length = %d\n\n", length_payload);

    {
        int rc = 1;

        uint16_t length = remaining_length;
        do {
            char d = (length % 128);
            length /= 128;
            /* if there are more digits to encode, set the top bit of this digit */
            if (length > 0) {
                d |= 0x80;
            }
            answer_msg[rc++] = (uint8_t)d;
        } while (length > 0);

        //answer_msg[1] = remaining_length;
        //answer_msg[2] = 0;
        //answer_msg[3] = length_topic_name;

        answer_msg[rc++] = length_topic_name >> 8;
        answer_msg[rc++] = length_topic_name & 0xFF;
    }

    rc += 3; // is the fixed header length


    for (i = 0; i < length_topic_name; i++){
        answer_msg[rc+i] = *(topic_name++);
    }
    for (i = 0; i < length_payload; i++) {
        answer_msg[length_topic_name+rc+i] = *(payload++);
        Serial.println((char)answer_msg[length_topic_name+rc+i]);
    }

    for (uint16_t f = 0; f < 10; f++) {
        Serial.println(answer_msg[f]);
    }

    delay(0);
    DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SENDMESSAGE]\n", num);
    DEBUG_MQTTBROKER_HEX((uint8_t *) &answer_msg, remaining_length + rc);

    Serial.println("\n\n\n");
    Serial.println(remaining_length + rc);
    Serial.println("\n\n\n");

    WS->sendBIN(num, (const uint8_t *) &answer_msg, remaining_length + rc - 2);
}

void MQTTbroker_lite::connect(uint8_t num) {
    MQTTclients[num].status = true;
}

uint16_t MQTTbroker_lite::MSB_LSB(uint8_t *msb_byte) {
    return (uint16_t) (*msb_byte << 8) + *(msb_byte + 1);
}

void MQTTbroker_lite::sendAnswer(uint8_t num, uint8_t fixed_header_comm, uint8_t fixed_header_lsb,
                                 uint8_t fixed_header_remaining_length, uint8_t *variable_header,
                                 uint8_t variable_header_length, uint8_t *payload, uint16_t payload_length) {
    uint8_t i;
    uint8_t answer_msg[fixed_header_remaining_length + 2];

    answer_msg[0] = (fixed_header_comm << 4) | fixed_header_lsb;
    answer_msg[1] = fixed_header_remaining_length;
    for (i = 2; i < variable_header_length + 2; i++) answer_msg[i] = *(variable_header++);

    switch (fixed_header_comm) {
        case CONNACK: //2
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SEND] CONNACK\n", num);
            break;
        case PUBACK: //4 QoS level 1
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SEND] PUBACK\n", num);
            break;
        case PUBREC: //5 QoS level 2, part 1
            break;
        case PUBREL: //6 QoS level 2, part 2
            break;
        case PUBCOMP: //7 QoS level 2, part 3
            break;
        case SUBACK: //9
            answer_msg[i] = *payload;
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SEND] SUBACK\n", num);
            break;
        case UNSUBACK: //11
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SEND] UNSUBACK\n", num);
            break;
        case PINGRESP: //13
            DEBUG_MQTTBROKER("[MQTT_BROKER][%d][SEND] PINGRESP\n", num);
            break;
        default:
            return;
    }
    DEBUG_MQTTBROKER_HEX((uint8_t *) &answer_msg, fixed_header_remaining_length + 2);

    delay(0);
    WS->sendBIN(num, (const uint8_t *) &answer_msg, fixed_header_remaining_length + 2);
}
