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
 */


#ifndef HOMEGENIE_MINI_MQTTCLIENT_H
#define HOMEGENIE_MINI_MQTTCLIENT_H

#include <mqtt_client.h>
#include <LinkedList.h>


#include "MQTTChannel.h"
#include "Task.h"
#include "data/Module.h"
#include "service/api/APIRequest.h"


#include "esp_crt_bundle.h"

// TODO: TLS/SSL support to be completed

static const char *mosquitto_org_pem PROGMEM = R"EOF(-----BEGIN CERTIFICATE-----
MIIEAzCCAuugAwIBAgIUBY1hlCGvdj4NhBXkZ/uLUZNILAwwDQYJKoZIhvcNAQEL
BQAwgZAxCzAJBgNVBAYTAkdCMRcwFQYDVQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwG
A1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1vc3F1aXR0bzELMAkGA1UECwwCQ0ExFjAU
BgNVBAMMDW1vc3F1aXR0by5vcmcxHzAdBgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hv
by5vcmcwHhcNMjAwNjA5MTEwNjM5WhcNMzAwNjA3MTEwNjM5WjCBkDELMAkGA1UE
BhMCR0IxFzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTES
MBAGA1UECgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVp
dHRvLm9yZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzCCASIwDQYJ
KoZIhvcNAQEBBQADggEPADCCAQoCggEBAME0HKmIzfTOwkKLT3THHe+ObdizamPg
UZmD64Tf3zJdNeYGYn4CEXbyP6fy3tWc8S2boW6dzrH8SdFf9uo320GJA9B7U1FW
Te3xda/Lm3JFfaHjkWw7jBwcauQZjpGINHapHRlpiCZsquAthOgxW9SgDgYlGzEA
s06pkEFiMw+qDfLo/sxFKB6vQlFekMeCymjLCbNwPJyqyhFmPWwio/PDMruBTzPH
3cioBnrJWKXc3OjXdLGFJOfj7pP0j/dr2LH72eSvv3PQQFl90CZPFhrCUcRHSSxo
E6yjGOdnz7f6PveLIB574kQORwt8ePn0yidrTC1ictikED3nHYhMUOUCAwEAAaNT
MFEwHQYDVR0OBBYEFPVV6xBUFPiGKDyo5V3+Hbh4N9YSMB8GA1UdIwQYMBaAFPVV
6xBUFPiGKDyo5V3+Hbh4N9YSMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQEL
BQADggEBAGa9kS21N70ThM6/Hj9D7mbVxKLBjVWe2TPsGfbl3rEDfZ+OKRZ2j6AC
6r7jb4TZO3dzF2p6dgbrlU71Y/4K0TdzIjRj3cQ3KSm41JvUQ0hZ/c04iGDg/xWf
+pp58nfPAYwuerruPNWmlStWAXf0UTqRtg4hQDWBuUFDJTuWuuBvEXudz74eh/wK
sMwfu1HFvjy5Z0iMDU8PUDepjVolOCue9ashlS4EB5IECdSR2TItnAIiIwimx839
LdUdRudafMu5T5Xma182OC0/u/xRlEm+tvKGGmfFcN0piqVl8OrSPBgIlb+1IKJE
m/XriWr/Cq4h/JfB7NTsezVslgkBaoU=
-----END CERTIFICATE-----)EOF";


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

        MQTTClient() {
            setLoopInterval(5000);
        };

        void loop() override {

            if (isEnabled && !clientStarted) {
                start();
            }

        }

        void configure(LinkedList<ModuleParameter*>& parameters) {
            auto address = String();
            auto port = String();
            auto tls = String();
            auto webSockets = String();
            auto username = String();
            auto password = String();
            for (ModuleParameter* p: parameters) {
                if(p->name.equals("ConfigureOptions.ServerAddress")) {
                    address = String(p->value);
                } else if (p->name.equals("ConfigureOptions.ServerPort")) {
                    port = String(p->value);
                } else if (p->name.equals("ConfigureOptions.TLS")) {
                    tls = String(p->value);
                } else if (p->name.equals("ConfigureOptions.WebSockets")) {
                    webSockets = String(p->value);
                } else if (p->name.equals("ConfigureOptions.Username")) {
                    username = String(p->value);
                } else if (p->name.equals("ConfigureOptions.Password")) {
                    password = String(p->value);
                }
            }

            auto brokerUrl = new String();
            if (tls.equals("on")) {
                *brokerUrl = webSockets.equals("on") ? "wss://" : "mqtts://";
            } else {
                *brokerUrl = webSockets.equals("on") ? "ws://" : "mqtt://";
            }
            *brokerUrl += address + String(":") + port;

            stop();

            mqtt_cfg.uri = brokerUrl->c_str();
            mqtt_cfg.username = username.c_str();
            mqtt_cfg.password = password.c_str();

        }

        void enable() {
            isEnabled = true;
        }
        void disable() {
            isEnabled = false;
            stop();
        }

        void start() {

            if (!clientStarted && ESP_WIFI_STATUS == WL_CONNECTED) {

                /*
                mbedtls_ssl_config conf;
                mbedtls_ssl_config_init(&conf);
                arduino_esp_crt_bundle_attach(&conf);
                */

                client = esp_mqtt_client_init(&mqtt_cfg);
                /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
                esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, nullptr);

                if (esp_mqtt_client_start(client) == ESP_OK) {
                    clientStarted = true;
                }
            }

        }

        void stop() {

            if (clientStarted) {
                esp_mqtt_client_stop(client);
                clientStarted = false;
            }

        }

        void broadcast(String *topic, String *payload) override {
            esp_mqtt_client_publish(client, topic->c_str(), payload->c_str(), (uint16_t)payload->length(), 0, 0);
        }

    private:
        bool clientStarted = false;
        bool isEnabled = false;
        esp_mqtt_client_handle_t client = nullptr;
        esp_mqtt_client_config_t mqtt_cfg { .uri = "" };

        /*
         * @brief Event handler registered to receive MQTT events
         *
         *  This function is called by the MQTT client event loop.
         *
         * @param handler_args user data registered to the event.
         * @param base Event base for the handler(always MQTT Base in this example).
         * @param event_id The id for the received event.
         * @param event_data The data for the event, esp_mqtt_event_handle_t.
         */
        static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
        {
            String topic = Config::system.id + "/#";

            // Event dispatched from event loop `base` with `event_id`
            auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
            esp_mqtt_client_handle_t client = event->client;
            int msg_id;
            switch ((esp_mqtt_event_id_t)event_id) {

                case MQTT_EVENT_CONNECTED:
                    esp_mqtt_client_subscribe(client, topic.c_str(), 1);
                    break;

                case MQTT_EVENT_DISCONNECTED:
                    break;
                case MQTT_EVENT_SUBSCRIBED:
                    break;
                case MQTT_EVENT_UNSUBSCRIBED:
                    break;
                case MQTT_EVENT_PUBLISHED:
                    break;

                case MQTT_EVENT_DATA: {
                    auto t = String(event->topic) + "/";
                    String cid;
                    String domain;
                    String address;
                    String recipient;
                    String type;
                    int i = t.indexOf('/');
                    while (i > 0) {
                        if (cid.isEmpty()) {
                            cid = t.substring(0, i);
                        } else
                        if (domain.isEmpty()) {
                            domain = t.substring(0, i);
                        } else
                        if (address.isEmpty()) {
                            address = t.substring(0, i);
                        } else
                        if (type.isEmpty()) {
                            type = t.substring(0, i);
                            break;
                        }
                        t = t.substring(i + 1);
                        i = t.indexOf('/');
                    }

                    if (cid == Config::system.id && domain == "MQTT.Listeners") {
                        if (address == Config::system.id) {
                            auto jsonRequest = String(event->data, event->data_len);
                            JsonDocument doc;
                            DeserializationError error = deserializeJson(doc, jsonRequest);
                            if (error.code()) break;
                            if (type == "command") {

                                auto apiUrl = String("/api/") + doc["Domain"].as<String>() + String("/") + doc["Address"].as<String>() + String("/") + doc["Command"].as<String>();
                                String data = "";
                                auto tid = doc["TransactionId"].as<String>();
                                if (requestHandler != nullptr) {
                                    requestHandler->onMqttRequest(nullptr, apiUrl, data, tid);
                                }

                            } else if (type == "request") {

                                auto apiUrl = String("/api/") + doc["Request"].as<String>();
                                auto data = doc["Data"].as<String>();
                                auto tid = doc["TransactionId"].as<String>();
                                if (requestHandler != nullptr) {
                                    requestHandler->onMqttRequest(nullptr, apiUrl, data, tid);
                                }

                            }
                        }
                    }
                } break;

                case MQTT_EVENT_ERROR:
                    /*
                    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                        ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
                        ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
                        ESP_LOGI(TAG, "Last captured errno : %d (%s)",  event->error_handle->esp_transport_sock_errno,
                                 strerror(event->error_handle->esp_transport_sock_errno));
                    } else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
                        ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
                    } else {
                        ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
                    }
                    break;
                    */
                default:
                    break;
            }
        }
    };

}

#endif //HOMEGENIE_MINI_MQTTCLIENT_H
