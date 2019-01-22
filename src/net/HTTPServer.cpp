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
 * - 2019-01-10 Initial release
 *
 */

#include <LinkedList.h>
#include "HTTPServer.h"

namespace Net {

    using namespace IO;

    static ESP8266WebServer httpServer(HTTP_SERVER_PORT);

    static LinkedList<WiFiClient> wifiClients;

    HTTPServer::HTTPServer() {

    }

    void HTTPServer::begin() {
        httpServer.on("/start.html", HTTP_GET, []() {
            httpServer.send(200, "text/plain", "Hello World!");
        });
        httpServer.on("/description.xml", HTTP_GET, []() {
            SSDP.schema(httpServer.client());
        });
        static HTTPServer* i = this;
        httpServer.on("/api/HomeAutomation.HomeGenie/Logging/RealTime.EventStream/", HTTP_GET, []() {
            WiFiClient sseClient = httpServer.client();
            wifiClients.add(sseClient);
            i->serverSentEventHeader(sseClient);
            sseClient.flush();
            // connection: CLOSE
        });
        httpServer.addHandler(this);

        httpServer.begin();
        Logger::info("|  ✔ HTTP service");

        SSDP.setSchemaURL("description.xml");
        SSDP.setHTTPPort(80);
        SSDP.setName("HomeGenie:mini V1.0");
        SSDP.setSerialNumber("ABC0123456789");
        SSDP.setURL("192.168.2.109");
        SSDP.setModelName("HomeGenie:mini 2019");
        SSDP.setModelNumber("2134567890");
        SSDP.setModelURL("http://homegenie.it");
        SSDP.setManufacturer("G-Labs");
        SSDP.setManufacturerURL("https://glabs.it");
        SSDP.begin();
        Logger::info("|  ✔ SSDP service");
    }

    void HTTPServer::loop() {
        Logger::verbose("%s loop() >> BEGIN", HTTPSERVER_LOG_PREFIX);
        httpServer.handleClient();
        Logger::verbose("%s loop() << END", HTTPSERVER_LOG_PREFIX);
    }

    // BEGIN RequestHandler interface methods
    bool HTTPServer::canHandle(HTTPMethod method, String uri) {
        return false;
    }
    bool HTTPServer::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        return false;
    }
    // END RequestHandler interface methods

    void HTTPServer::addHandler(RequestHandler* handler) {
        httpServer.addHandler(handler);
    }

    void HTTPServer::sendSSEvent(String p, String v) {
        // send message to all connected clients and remove disconnected ones
        for (int c = wifiClients.size()-1; c >= 0; c--) {
            auto sseClient = wifiClients.get(c);
            if (sseClient.connected()) {
                serverSentEvent(sseClient, p, v);
            } else wifiClients.remove(c);
        }
    }

    void HTTPServer::serverSentEventHeader(WiFiClient client) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/event-stream;charset=UTF-8");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
        client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
        client.println();
        client.flush();
    }

    void HTTPServer::serverSentEvent(WiFiClient client, String event, String value) {
        // id: 1548081759906.19
        // data: {"Timestamp":"2019-01-21T14:42:39.906194Z","UnixTimestamp":1548081759906.19,"Domain":"HomeAutomation.ZWave","Source":"7","Description":"ZWave Node","Property":"Meter.Watts","Value":0}
        client.printf("id: %lu\n", millis());
        client.printf(R"(data: {"Timestamp":"2019-01-21T14:42:39.906194Z","UnixTimestamp":1548081759906.19,"Description":"","Domain":"HomeAutomation.HomeGenie","Source":"mini","Property":"%s","Value":"%s"})", event.c_str(), value.c_str());
        client.println();
        client.println();
        client.flush();
    }
}
