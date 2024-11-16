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

#include "HTTPServer.h"

#include "net/NetManager.h"
#include "service/EventRouter.h"

namespace Net {

    using namespace IO;
    using namespace Service;

    const char SSDP_Name[] PROGMEM = CONFIG_DEVICE_MODEL_NAME;
    const char SSDP_SerialNumber[] PROGMEM = CONFIG_DEVICE_SERIAL_NUMBER;
    const char SSDP_ModelName[] PROGMEM = CONFIG_DEVICE_MODEL_NAME;
    const char SSDP_ModelNumber[] PROGMEM = CONFIG_DEVICE_MODEL_NUMBER;
    const char SSDP_ModelDescription[] PROGMEM = "HomeGenie Mini Device";
    const char SSDP_ModelURL[] PROGMEM = "https://homegenie.it";
    const char SSDP_Manufacturer[] PROGMEM = "G-Labs";
    const char SSDP_ManufacturerURL[] PROGMEM = "https://github.com/genielabs";

    static WebServer httpServer(HTTP_SERVER_PORT);

    LinkedList<WiFiClient> wifiClients;
#ifndef DISABLE_SSE
    LinkedList<QueuedMessage> events;
#endif

    String ipAddress;

    void HTTPServer::begin() {
        //httpServer.on("/start.html", HTTP_GET, []() {
        //    httpServer.send(200, "text/plain", "Hello World!");
        //});

        //httpServer.on("/description.xml", HTTP_GET, []() {
        //    SSDPDevice.schema(httpServer.client());
        //});

#ifndef DISABLE_SSE
        static HTTPServer* i = this;
        httpServer.on("/api/HomeAutomation.HomeGenie/Logging/RealTime.EventStream/", HTTP_GET, []() {
            i->sseClientAccept();
        });
        // alias of "../Logging/RealTime.EventStream"
        httpServer.on("/events", HTTP_GET, []() {
            i->sseClientAccept();
        });
        httpServer.addHandler(this);
#endif

        httpServer.begin();
        Logger::info("|  ✔ HTTP service");

//        ipAddress = WiFi.localIP().toString();
    }

    void HTTPServer::loop() {
        Logger::verbose("%s loop() >> BEGIN", HTTPSERVER_LOG_PREFIX);

        String localIP = WiFi.localIP().toString();
        if (!ipAddress.equals(localIP) && !localIP.equals("0.0.0.0") && !localIP.isEmpty()) {
            // New IP address
            ipAddress = localIP;
            //Logger::info("|  ✔ New IP address %s", ipAddress.c_str());

            // SSDP UDN uuid
            Config::system.id = SSDPDevice.getId();
            //Logger::info("|  ✔ SSDP UDN uuid: ", Config::system.id.c_str());

            String ssdpUri = Config::system.id + String(".xml");
            SSDPDevice.setSchemaURL(FPSTR(ssdpUri.c_str()));
            SSDPDevice.setName(FPSTR(SSDP_Name));
            SSDPDevice.setSerialNumber(FPSTR(SSDP_SerialNumber));
            SSDPDevice.setURL(ipAddress);
            SSDPDevice.setModelName(FPSTR(SSDP_ModelName));
            SSDPDevice.setModelNumber(FPSTR(SSDP_ModelNumber));
            SSDPDevice.setModelDescription(FPSTR(SSDP_ModelDescription));
            SSDPDevice.setModelURL(FPSTR(SSDP_ModelURL));
            SSDPDevice.setManufacturer(FPSTR(SSDP_Manufacturer));
            SSDPDevice.setManufacturerURL(FPSTR(SSDP_ManufacturerURL));
            SSDPDevice.setHTTPPort(80);

            Logger::info("|  ✔ SSDP service");

            SSDPDevice.setFriendlyName(Config::system.friendlyName);
            Logger::info("|  ✔ UPnP friendly name: %s", Config::system.friendlyName.c_str());
        }

        httpServer.handleClient();
        SSDPDevice.handleClient();

#ifndef DISABLE_SSE
        if (events.size() > 0) {

            auto e = events.pop();
            // send message to all connected clients and remove disconnected ones
            for (int c = wifiClients.size() - 1; c >= 0; c--) {
                auto sseClient = wifiClients.get(c);
                if (sseClient.connected()) {
                    serverSentEvent(sseClient, e.domain, e.sender, e.event, e.value);
                } else wifiClients.remove(c);
            }

        }
#endif
        Logger::verbose("%s loop() << END", HTTPSERVER_LOG_PREFIX);
    }

    void HTTPServer::addHandler(RequestHandler* handler) {
        httpServer.addHandler(handler);
    }

#ifndef DISABLE_SSE
    // BEGIN RequestHandler interface methods
#if ESP_ARDUINO_VERSION_MAJOR > 2
    bool HTTPServer::canHandle(HTTPMethod method, const String& uri) {
#else
    bool HTTPServer::canHandle(HTTPMethod method, String uri) {
#endif
        String ssdpUri = String("/") + Config::system.id + String(".xml");
        if (uri == ssdpUri) {
            return true;
        }
        return false;
    }

#if ESP_ARDUINO_VERSION_MAJOR > 2
    bool HTTPServer::handle(WebServer& server, HTTPMethod requestMethod, const String& requestUri) {
#else
    bool HTTPServer::handle(WebServer& server, HTTPMethod requestMethod, String requestUri) {
#endif
        String ssdpUri = String("/") + Config::system.id + String(".xml");
        if (requestUri == ssdpUri) {
            SSDPDevice.schema(httpServer.client());
            return true;
        }
        return false;
    }
    // END RequestHandler interface methods

    void HTTPServer::sendSSEvent(String domain, String address, String event, String value) {
        auto m = QueuedMessage(domain, address, event, value, nullptr, IOEventDataType::Undefined);
        events.add(m);
    }

    void HTTPServer::serverSentEventHeader(WiFiClient &client) {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/event-stream;charset=UTF-8");
        client.println("Connection: close");  // the connection will be closed after completion of the response
        client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
        client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
        client.println();
        //client.flush();
    }

    void HTTPServer::serverSentEvent(WiFiClient &client, String &domain, String &address, String &event, String &value) {
        // id: 1548081759906.19
        // data: {"Timestamp":"2019-01-21T14:42:39.906194Z","UnixTimestamp":1548081759906.19,"Domain":"HomeAutomation.ZWave","Source":"7","Description":"ZWave Node","Property":"Meter.Watts","Value":0}
        String date = Net::TimeClient::getTimeClient().getFormattedDate();
        unsigned long epoch = Net::TimeClient::getTimeClient().getEpochTime();
        int ms = Net::TimeClient::getTimeClient().getMilliseconds();
        client.printf("id: %lu\n", millis());
        client.printf(R"(data: {"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                date.c_str(), epoch, ms, domain.c_str(), address.c_str(), event.c_str(), value.c_str());
        client.println();
        client.println();
        //client.flush();
    }

    void HTTPServer::sseClientAccept() {
        WiFiClient sseClient = httpServer.client();
        wifiClients.add(sseClient);
        this->serverSentEventHeader(sseClient);
        //sseClient.flush();
        // connection: CLOSE
    }
#endif
}
