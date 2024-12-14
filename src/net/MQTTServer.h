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
 * - 2019-01-14 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_MQTTSERVER_H
#define HOMEGENIE_MINI_MQTTSERVER_H

#include <ArduinoJson.h>
#include <io/Logger.h>
#include <WiFiServer.h>

#include "Task.h"
#include "net/mqtt/MQTTBrokerMini.h"

#include "./mqtt/MQTTChannel.h"

namespace Net {
    using namespace MQTT;
    using namespace IO;

    typedef std::function<void(uint8_t num, const char* domain, const char* address, const char* command)> ApiRequestEvent;

    /// Simple MQTT Broker implementation over WebSockets
    class MQTTServer : MQTTChannel, Task {
    public:
        void begin();
        void loop() override;

        void onRequest(ApiRequestEvent cb) {
            apiCallback = cb;
        }

        void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
        void mqttCallback(uint8_t num, const Events_t* event, const String* topic_name, uint8_t* payload, uint16_t length_payload);

        void broadcast(String *topic, String *payload) override;
        void broadcast(uint8_t num, String *topic, String *payload) override;

    private:
        WebSocketsServer* webSocket = nullptr;
        MQTTBrokerMini* mqttBroker = nullptr;
        ApiRequestEvent apiCallback = nullptr;
    };

}

#endif //HOMEGENIE_MINI_MQTTSERVER_H
