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

#ifndef HOMEGENIE_MINI_MQTTSERVER_H
#define HOMEGENIE_MINI_MQTTSERVER_H

#include <Arduino.h>

#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <MQTTbroker_lite.h>
#include <Task.h>

namespace Net {

/// Simple MQTT Broker implementation over WebSockets
    class MqttServer : Task {
    public:
        void begin();
        void loop();
        static void webSocketEventStatic(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
        static void mqttCallbackStatic(uint8_t num, Events_t event, String topic_name, uint8_t * payload, uint8_t length_payload);
    private:
        WebSocketsServer *webSocket;
        MQTTbroker_lite *mqttBroker;
    };

}

#endif //HOMEGENIE_MINI_MQTTSERVER_H
