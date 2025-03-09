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
 */


#ifndef HOMEGENIE_MINI_MQTTCLIENT_H
#define HOMEGENIE_MINI_MQTTCLIENT_H

#include <mbedtls/base64.h>
#include <mqtt_client.h>
#include <LinkedList.h>
#include <UUID.h>

#include "Task.h"
#include "data/Module.h"
#include "service/api/APIRequest.h"

#include "./mqtt/MQTTChannel.h"

#include "esp_crt_bundle.h"

// TODO: TLS/SSL support to be completed

namespace Net {
    using namespace Data;
    using namespace Service::API;

    // NetRequestHandler interface
    class MQTTRequestHandler {
    public:
        virtual bool onMqttRequest(void* sender, String&, String&, String&) = 0; // pure virtual
    };

    class MQTTClient: MQTTChannel, Task {
    public:
        static MQTTRequestHandler* requestHandler;

        MQTTClient();

        void loop() override;

        void configure(LinkedList<ModuleParameter*>& parameters);

        void enable();
        void disable();

        void start();
        void stop();

        void broadcast(String *topic, String *payload) override;
        void broadcast(uint8_t* topic, uint16_t topic_length, uint8_t* byte_payload, size_t length) override;

        static void encrypt(String *topic, String *payload);
        static void encryptTopic(String* topic);
        static void decryptTopic(String* topic);
        static void encryptionFilter(String* payload);

    private:
        static String encryptionKey;
        static bool enableEncryption;
        static bool isConnected;
        String clientId;
        String brokerUrl;
        String username;
        String password;
        bool isEnabled = false;
        bool stopRequested = false;
        bool clientStarted = false;
        esp_mqtt_client_handle_t client = nullptr;
#if ESP_IDF_VERSION_MAJOR >= 5
        esp_mqtt_client_config_t mqtt_cfg {};
#else
        esp_mqtt_client_config_t mqtt_cfg { .uri = "" };
#endif
        static void xorFilter(String* payload, String* clientKey = 0);
        static void andFilter(String* payload, String* clientKey = 0);

        static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    };

}

#endif //HOMEGENIE_MINI_MQTTCLIENT_H
