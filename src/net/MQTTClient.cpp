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


#include "defs.h"

#ifndef DISABLE_MQTT_CLIENT

#include "MQTTClient.h"

namespace Net {

    MQTTRequestHandler* MQTTClient::requestHandler = nullptr;
    bool MQTTClient::enableEncryption = false;
    bool MQTTClient::isConnected = false;

    MQTTClient::MQTTClient() {
        setLoopInterval(2000);
    }

    void MQTTClient::loop() {

        if (stopRequested) {
            stop();
            stopRequested = false;
        } else if (isEnabled && !clientStarted) {
            start();
        }

    }

    void MQTTClient::configure(LinkedList<ModuleParameter *> &parameters) {
        auto address = String();
        auto port = String();
        auto tls = String();
        auto webSockets = String();
        username = "";
        password = "";
        encryptionKey = "";
        enableEncryption = false;
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
            } else if (p->name.equals("ConfigureOptions.Encryption")) {
                String enable = String(p->value);
                enable.toLowerCase();
                enableEncryption = enable.equals("on");
            } else if (p->name.equals("ConfigureOptions.EncryptionKey")) {
                encryptionKey = String(p->value);
            }
        }

        if (tls.equals("on")) {
            brokerUrl = webSockets.equals("on") ? "wss://" : "mqtts://";
        } else {
            brokerUrl = webSockets.equals("on") ? "ws://" : "mqtt://";
        }
        brokerUrl += address + String(":") + port;

        UUID uuid;
        uint32_t seed1 = random(999999999);
        uint32_t seed2 = random(999999999);
        uuid.seed(seed1, seed2);
        uuid.generate();
        clientId = uuid.toCharArray();
#if ESP_IDF_VERSION_MAJOR >= 5
        mqtt_cfg.credentials.client_id = clientId.c_str();
        mqtt_cfg.broker.address.uri = brokerUrl.c_str();
        mqtt_cfg.credentials.username = username.c_str();
        mqtt_cfg.credentials.authentication.password = password.c_str();
#else
        mqtt_cfg.client_id = clientId.c_str();
        mqtt_cfg.uri = brokerUrl.c_str();
        mqtt_cfg.username = username.c_str();
        mqtt_cfg.password = password.c_str();
#endif
        stopRequested = true;
    }

    void MQTTClient::enable() {
        isEnabled = true;
    }

    void MQTTClient::disable() {
        isEnabled = false;
        stopRequested = true;
    }

    void MQTTClient::start() {

        if (!clientStarted && ESP_WIFI_STATUS == WL_CONNECTED) {
            esp_mqtt_client_destroy(client);
            client = esp_mqtt_client_init(&mqtt_cfg);
            if (client != nullptr) {
                /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
                esp_mqtt_client_register_event(client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID),
                                               mqtt_event_handler, nullptr);

                if (esp_mqtt_client_start(client) == ESP_OK) {
                    clientStarted = true;
                }
            }
        }

    }

    void MQTTClient::stop() {

        if (clientStarted) {
            esp_mqtt_client_disconnect(client);
            esp_mqtt_client_stop(client);
            clientStarted = false;
        }

    }

    void MQTTClient::broadcast(String *topic, String *payload) {
        if (!clientStarted || !isConnected) return;
        encrypt(topic, payload);
        esp_mqtt_client_publish(client, topic->c_str(), payload->c_str(), static_cast<uint16_t>(payload->length()), 0, 0);
    }

    void MQTTClient::broadcast(uint8_t* topic, uint16_t topic_length, uint8_t *byte_payload, size_t length) {
        if (!clientStarted || !isConnected) return;
        auto payload = String(byte_payload, length);
        auto t = String(topic, topic_length);
        encrypt(&t, &payload);
        esp_mqtt_client_publish(client, t.c_str(), payload.c_str(), static_cast<uint16_t>(length), 0, 0);
    }

    void MQTTClient::encrypt(String *topic, String *payload) {
        if (enableEncryption) {
            int ci = topic->indexOf("/");
            if (ci > 0) {
                auto cid = topic->substring(0, ci);
                auto cmd = topic->substring(ci + 1);
                encryptTopic(&cid);
                encryptTopic(&cmd);
                *topic = cid + String("/") + cmd;
                decryptTopic(&cmd);
            }
            encryptionFilter(payload);
        }
    }

    void MQTTClient::encryptTopic(String *topic) {
        encryptionFilter(topic);

        size_t encLength = topic->length() * 4;
        unsigned char encryptedTopic[encLength];
        size_t length;
        mbedtls_base64_encode(encryptedTopic, encLength, &length,
                              (const uint8_t *)topic->c_str(), topic->length());

        *topic = String(encryptedTopic, length);
        topic->replace("/", "%");
        topic->replace("+", "&");
    }

    void MQTTClient::decryptTopic(String *topic) {
        topic->replace("%", "/");
        topic->replace("&", "+");

        size_t encLength = topic->length();
        uint8_t encryptedTopic[encLength];
        size_t length;
        if (mbedtls_base64_decode(encryptedTopic, encLength, &length,
                                  (const uint8_t*)topic->c_str(),
                                  topic->length()) == 0) {
            *topic = String(encryptedTopic, length);
            encryptionFilter(topic);
        }
    }

    void MQTTClient::encryptionFilter(String *payload) {
        if (!encryptionKey.isEmpty()) {
            uint8_t output[256];
            size_t length;
            mbedtls_base64_decode(output, 256, &length, (const uint8_t*)encryptionKey.c_str(), encryptionKey.length());
            auto s = String((const char*)output, length);
            andFilter(&s, &Config::system.id);
            xorFilter(payload, &s);
        }
    }

    void MQTTClient::xorFilter(String *payload, String *clientKey) {
        if (!clientKey->isEmpty()) {
            for (int c = 0; c < payload->length(); c++) {
                auto key = clientKey->charAt(c % clientKey->length());
                auto in = (byte) payload->charAt(c);
                auto out = key ^ in;
                payload->setCharAt(c, out);
            }
        }
    }

    void MQTTClient::andFilter(String *payload, String *clientKey) {
        if (!clientKey->isEmpty()) {
            for (int c = 0; c < payload->length(); c++) {
                auto key = clientKey->charAt(c % clientKey->length());
                auto in = (byte) payload->charAt(c);
                auto out = key & in;
                payload->setCharAt(c, out);
            }
        }
    }

    void MQTTClient::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

        // Event dispatched from event loop `base` with `event_id`
        auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
        esp_mqtt_client_handle_t client = event->client;
        int msg_id;
        switch ((esp_mqtt_event_id_t)event_id) {

            case MQTT_EVENT_CONNECTED: {
                if (enableEncryption) {
                    auto cid = String(Config::system.id);
                    encryptTopic(&cid);
                    cid += "/#";
                    esp_mqtt_client_subscribe(client, cid.c_str(), 1);
                } else {
                    String topic = Config::system.id + "/#";
                    esp_mqtt_client_subscribe(client, topic.c_str(), 1);
                }
                isConnected = true;
            } break;

            case MQTT_EVENT_DISCONNECTED:
                isConnected = false;
                break;
            case MQTT_EVENT_SUBSCRIBED:
                break;
            case MQTT_EVENT_UNSUBSCRIBED:
                break;
            case MQTT_EVENT_PUBLISHED:
                break;

            case MQTT_EVENT_DATA: {
                auto t = String(event->topic);
                if (enableEncryption) {
                    int i = t.indexOf('/');
                    if (i > 0) {
                        auto cid = t.substring(0, i);
                        decryptTopic(&cid);
                        auto payload = t.substring(i + 1);
                        decryptTopic(&payload);
                        t = cid + "/" + payload;
                    } else {
                        decryptTopic(&t);
                    }
                }
                t += "/";
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
                        if (enableEncryption) {
                            encryptionFilter(&jsonRequest);
                        }
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
                isConnected = false;
            default:
                break;
        }
    }

    String MQTTClient::encryptionKey;
}

#endif