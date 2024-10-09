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
 * - 2019-01-10 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_NETMANAGER_H
#define HOMEGENIE_MINI_NETMANAGER_H

#ifdef ESP8266
#include <ESP8266mDNS.h>
#else
#include <ESPmDNS.h>
#endif
#include <WiFiServer.h>

#include <LinkedList.h>
#include <MsgPack.h>
#include <WebSocketsServer.h>

#include "Config.h"
#include "net/HTTPServer.h"
#include "net/WiFiManager.h"

#ifndef DISABLE_MQTT
#include "net/MQTTServer.h"
#endif

#ifndef DISABLE_BLUETOOTH
#include <net/BluetoothManager.h>
#endif

#include "io/IOEventDomains.h"

#include "TimeClient.h"

#define NETMANAGER_LOG_PREFIX           "@Net::NetManager"

namespace Net {

    // ResponseCallback
    class ResponseCallback {
    public:
        unsigned int contentLength = 0;
        virtual void beginGetLength() = 0;
        virtual void endGetLength() = 0;
        virtual void write(const char* s) = 0;
        virtual void writeAll(const char* s) = 0;
        virtual void error(const char* s) = 0;
    };

    class DummyResponseCallback : public ResponseCallback {
    public:
        void beginGetLength() override {};
        void endGetLength() override {};
        void write(const char* s) override {};
        void writeAll(const char* s) override {};
        void error(const char* s) override {};
    };

#ifndef DISABLE_MQTT
    class MQTTResponseCallback : public ResponseCallback {
    public:
        MQTTResponseCallback(MQTTServer *server, uint8_t clientId, String* destinationTopic) {
            mq = server;
            cid = clientId;
            topic = destinationTopic;
        }
        void beginGetLength() override {
            buffer = "";
        };
        void endGetLength() override {
            mq->broadcast(topic, &buffer);
        };
        void write(const char* s) override {
            buffer += s;
        };
        void writeAll(const char* s) override {};
        void error(const char* s) override {};
    private:
        MQTTServer* mq;
        uint8_t cid;
        String* topic;
        String buffer;
    };
#endif

    // WebSocketResponseCallback
    class WebSocketResponseCallback : public ResponseCallback {
    public:
        WebSocketResponseCallback(WebSocketsServer *websocket, uint8_t clientId, String* rid) {
            ws = websocket;
            cid = clientId;
            requestId = rid;
        }
        void beginGetLength() override {};
        void endGetLength() override {};
        void write(const char* s) override {};
        void writeAll(const char* s) override {
            if (requestId != nullptr) {
                //auto date = TimeClient::getTimeClient().getFormattedDate();
                unsigned long epoch = TimeClient::getTimeClient().getEpochTime();
                int ms = TimeClient::getTimeClient().getMilliseconds();
                //"#", requestId, "", "Response.Data", migRequest.ResponseData
                // Send as clear text
                //int sz = 1+snprintf(nullptr, 0, R"(data: {"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                //                    date.c_str(), epoch, ms, "#", requestId->c_str(), "Response.Data", s);
                //char msg[sz];
                //snprintf(msg, sz, R"({"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                //         date.c_str(), epoch, ms, "#", requestId->c_str(), "Response.Data", s);
                //ws->sendTXT(cid, msg);

                // Send binary packed message

                MsgPack::Packer packer;
                struct timeval tv_now{};
                gettimeofday(&tv_now, nullptr);
                MsgPack::object::timespec t = {
                        .tv_sec  = tv_now.tv_sec, /* int64_t  */
                        .tv_nsec = static_cast<uint32_t>(tv_now.tv_usec) / 10000  /* uint32_t */
                };
                packer.packTimestamp(t);
                auto epochs = String(epoch) + ms;
                packer.packFloat((epoch * 1000.0f) + ms);
                packer.pack(F("#"));
                packer.pack(requestId->c_str());
                packer.pack("");
                packer.pack(F("Response.Data"));
                packer.pack(s);
                ws->sendBIN(cid, packer.data(), packer.size());
                packer.clear();

                requestId->clear();
                requestId = nullptr;
            }
        };
        void error(const char* s) override {};
    private:
        WebSocketsServer* ws;
        uint8_t cid;
        String* requestId;
    };

    // WebServerResponseCallback
    class WebServerResponseCallback : public ResponseCallback {
    private:
        WebServer* server;
        bool disableOutput = false;
        void sendHeaders() {
            server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server->sendHeader("Pragma", "no-cache");
            server->sendHeader("Expires", "0");
            server->setContentLength(contentLength);
            server->send(200, "application/json; charset=utf-8", "");
        }
    public:
        WebServerResponseCallback(WebServer* server) {
            this->server = server;
        }
        void beginGetLength() override {
            contentLength = 0;
            disableOutput = true;
        }
        void endGetLength() override {
            sendHeaders();
            disableOutput = false;
        }
        void write(const char* s) override {
            contentLength += strlen(s);
            if (server != nullptr && !disableOutput) {
                server->sendContent(s);
            }
        }
        void writeAll(const char* s) override {
            contentLength += strlen(s);
            if (server != nullptr && !disableOutput) {
                sendHeaders();
                server->sendContent(s);
            }
        }
        void error(const char* s) override {
            server->send(400, "application/json", s);
        }
    };


    // NetRequestHandler interface
    class NetRequestHandler {
    public:
        virtual bool onNetRequest(void* sender, const char* requestMessage, ResponseCallback* responseCallback) = 0; // pure virtual
    };


    /// Network services management
    class NetManager : Task, RequestHandler {
    public:
        NetManager();
        ~NetManager();

        void begin();
        void loop() override;

        NTPClient& getTimeClient();
#ifndef DISABLE_BLUETOOTH
        BluetoothManager& getBLEManager();
#endif
        WiFiManager& getWiFiManager();
        HTTPServer& getHttpServer();

#ifndef DISABLE_MQTT
        MQTTServer& getMQTTServer();
#endif
        WebSocketsServer& getWebSocketServer();


        void setRequestHandler(NetRequestHandler *);

        // BEGIN HTTP RequestHandler interface methods

#if ESP_ARDUINO_VERSION_MAJOR > 2
        bool canHandle(HTTPMethod method, const String& uri) override {
#else
        bool canHandle(HTTPMethod method, String uri) override {
#endif
            return uri != nullptr && uri.startsWith("/api/");
        }
#if ESP_ARDUINO_VERSION_MAJOR > 2
        bool handle(WebServer &server, HTTPMethod requestMethod, const String& requestUri) override {
#else
        bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri) override {
#endif
            // append POST data to requestUri
            String uri = requestUri;
            for (int a = 0; a < server.args(); a++) {
                if (server.argName(a).length() > 0) {
                    uri += String("/") + server.argName(a);
                }
                uri += String ("/") + server.arg(a);
            }
            // create response callback
            auto responseCallback = new WebServerResponseCallback(&server);
            if (!netRequestHandler->onNetRequest(&server, uri.c_str(), responseCallback)) {
                responseCallback->error("Invalid request.");
            };
            delete responseCallback;
            return true;
        }

        // END HTTP RequestHandler interface methods

    private:
#ifndef DISABLE_BLUETOOTH
        BluetoothManager *bluetoothManager;
#endif
        WiFiManager *wiFiManager;
        HTTPServer *httpServer;
#ifndef DISABLE_MQTT
        MQTTServer *mqttServer;
#endif
        WebSocketsServer *webSocket;
        NetRequestHandler* netRequestHandler;

        TimeClient* timeClient;

    };

}

#endif //HOMEGENIE_MINI_NETMANAGER_H
