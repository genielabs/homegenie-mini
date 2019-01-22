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
 * Based on: https://github.com/xDWart/MQTTbroker
 *
 * Releases:
 * - Fixed bug in MQTT message header encoding/decoding that was causing random client disconnects (unsupported MQTT packet type 0)
 * - Increased topic/payload size limit from 127 bytes to 16384 bytes
 * - 2019-14-01 Initial release
 *
 */

#ifndef MQTTBROKER_H_
#define MQTTBROKER_H_

#include <Arduino.h>
#include <WebSocketsServer.h>

#include <io/Logger.h>

#define MQTT_VERSION_3_1_1              4

#define MQTTBROKER_NS_PREFIX           "Net::MQTT::MQTTBrokerMini"
#define MQTTBROKER_CLIENT_MAX           WEBSOCKETS_SERVER_CLIENT_MAX
#define MQTTBROKER_LOCAL_CLIENT_ID      MQTTBROKER_CLIENT_MAX
#define MQTTBROKER_VHEADER_MIN_LENGTH   3

#define DEBUG_MQTTBROKER(...) IO::Logger::traceN(__VA_ARGS__)
#define DEBUG_MQTTBROKER_HEX(...)

namespace Net { namespace MQTT {

        enum { //PACKETS
            CONNECT = 1,
            CONNACK = 2,
            PUBLISH = 3,
            PUBACK = 4,
            PUBREC = 5,
            PUBREL = 6,
            PUBCOMP = 7,
            SUBSCRIBE = 8,
            SUBACK = 9,
            UNSUBSCRIBE = 10,
            UNSUBACK = 11,
            PINGREQ = 12,
            PINGRESP = 13,
            DISCONNECT = 14,
        };

        enum { //Connect Flags
            CLEAN_SESSION = 0x02,
            WILL_FLAG = 0x04,
            WILL_QOS = 0x18,
            WILL_RETAIN = 0x20,
            PASSWORD_FLAG = 0x40,
            USER_NAME_FLAG = 0x80,
        };

        enum { //CONNACK_SP
            SESSION_PRESENT_ZERO = 0, //If the Server accepts a connection with CleanSession set to 1, the Server MUST set Session Present to 0 in the CONNACK packet
            SESSION_PRESENT_ONE = 1,  //look 3.2.2.2 Session Present [mqtt-v.3.1.1-os.pdf]
        };

        enum { //CONNACK_RC
            CONNECT_ACCEPTED = 0,    //"Connection Accepted"
            CONNECT_REFUSED_UPV = 1, //"Connection Refused: unacceptable protocol version"
            CONNECT_REFUSED_IR = 2,  //"Connection Refused: identifier rejected"
            CONNECT_REFUSED_SU = 3,  //"Connection Refused: server unavailable"
            CONNECT_REFUSED_BUP = 4, //"Connection Refused: bad user name or password"
            CONNECT_REFUSED_NA = 5,  //"Connection Refused: not authorized"
        };

        typedef struct {
            // TODO: add subscribed channels list and maybe some other useful info
            bool status;
        } MQTTBrokerClient_t;

        typedef enum {
            EVENT_CONNECT,
            EVENT_SUBSCRIBE,
            EVENT_PUBLISH,
            EVENT_DISCONNECT,
        } Events_t;

        typedef void(*callback_t)(uint8_t num, Events_t event, String topic_name, uint8_t *payload,
                                  uint16_t length_payload);

        class MQTTBrokerMini {
        public:
            MQTTBrokerMini(WebSocketsServer *webSocket);

            void begin(void);

            void setCallback(callback_t cb);

            void unsetCallback(void);

            void parsing(uint8_t num, uint8_t *payload, uint16_t length);

            void publish(uint8_t num, String topic, uint8_t *payload, uint16_t length);

            void broadcast(uint8_t num, String topic_name, uint8_t *payload, uint16_t length_payload);
            void broadcast(String topic_name, uint8_t *payload, uint16_t length_payload) {
                broadcast(MQTTBROKER_LOCAL_CLIENT_ID, topic_name, payload, length_payload);
            };

            void disconnect(uint8_t num);

            bool clientIsConnected(uint8_t num);

            MQTTBrokerClient_t *getClients();

        private:
            WebSocketsServer *WS;
            callback_t callback;

            void runCallback(uint8_t num, Events_t event, uint8_t *topic_name = NULL, uint16_t length_topic_name = 0,
                             uint8_t *payload = NULL, uint16_t length_payload = 0);

            void sendAnswer(uint8_t num, uint8_t fixed_header_comm, uint8_t fixed_header_lsb = 0,
                            uint8_t fixed_header_remaining_length = 0, uint8_t *variable_header = NULL,
                            uint8_t variable_header_length = 0, uint8_t *payload = NULL, uint16_t payload_length = 0);

            void sendMessage(uint8_t num, uint8_t *topic_name, uint16_t length_topic_name, uint8_t *payload,
                             uint16_t length_payload);

            void connect(uint8_t num);

            bool numIsIncorrect(uint8_t num);

            String data_to_string(uint8_t *data, uint16_t length);

            uint16_t MSB_LSB(uint8_t *msb_byte);
        };

}}

#endif /* MQTTBROKER_H_ */
