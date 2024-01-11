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
#ifdef ESP32
#include <ESP32Time.h>
#endif
#endif
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include <LinkedList.h>
#include <NTPClient.h>
#include <MsgPack.h>

#include "Config.h"
#include "net/HTTPServer.h"
#include "net/MQTTServer.h"
#include "net/WiFiManager.h"

#ifndef DISABLE_BLE
#include <net/BLEManager.h>
#endif
#ifndef CONFIGURE_WITH_WPA
#include <BluetoothSerial.h>
#include <Preferences.h>
#endif

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

    // WebSocketResponseCallback
    class WebSocketResponseCallback : public ResponseCallback {
    public:
        void beginGetLength() override {};
        void endGetLength() override {};
        void write(const char* s) override {};
        void writeAll(const char* s) override {};
        void error(const char* s) override {};
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
        bool begin();
        void loop() override;
#ifndef DISABLE_BLE
        BLEManager& getBLEManager();
#endif
        WiFiManager& getWiFiManager();
        HTTPServer& getHttpServer();
        MQTTServer& getMQTTServer();
        WebSocketsServer& getWebSocketServer();
        static NTPClient& getTimeClient();

        void setRequestHandler(NetRequestHandler *);

        // BEGIN HTTP RequestHandler interface methods

        bool canHandle(HTTPMethod method, String uri) override {
            return uri != nullptr && uri.startsWith("/api/");
        }
        bool handle(WebServer &server, HTTPMethod requestMethod, String requestUri) override {
            auto responseCallback = new WebServerResponseCallback(&server);
            if (!netRequestHandler->onNetRequest(&server, requestUri.c_str(), responseCallback)) {
                responseCallback->error("Invalid request.");
            };
            return true;
        }

        // END HTTP RequestHandler interface methods

    private:
#ifndef DISABLE_BLE
        BLEManager *bleManager;
#endif
        WiFiManager *wiFiManager;
        HTTPServer *httpServer;
        MQTTServer *mqttServer;
        WebSocketsServer *webSocket;
        NetRequestHandler* netRequestHandler;
    };

}

#endif //HOMEGENIE_MINI_NETMANAGER_H
