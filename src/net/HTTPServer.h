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

#ifndef HOMEGENIE_MINI_HTTPSERVER_H
#define HOMEGENIE_MINI_HTTPSERVER_H

#ifdef ESP8266
#define WebServer ESP8266WebServer
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif

#include <detail/RequestHandler.h>
#include <LinkedList.h>

#include "Task.h"

#include "SSDPDevice.h"

#define HTTPSERVER_LOG_PREFIX           "@Net::HTTPServer"
#define HTTP_SERVER_PORT 80

namespace Net {

    /// Implements HTTP and SSDP services
    class HTTPServer : Task, RequestHandler {
    public:
        void begin();
        void loop() override;
        void addHandler(RequestHandler* handler);
        // RequestHandler interface methods
        bool canHandle(HTTPMethod method, String uri) override;
        bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) override;
        void sendSSEvent(String domain, String address, String event, String value);
    private:
        void sseClientAccept();
        void serverSentEventHeader(WiFiClient &client);
        void serverSentEvent(WiFiClient &client, String &domain, String &address, String &event, String &value);
    };

}

#endif //HOMEGENIE_MINI_HTTPSERVER_H
