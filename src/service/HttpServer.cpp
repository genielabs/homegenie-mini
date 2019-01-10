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
 * - 2019-10-01 Initial release
 *
 */

#include "HttpServer.h"

namespace Service {

    using namespace IO;

    ESP8266WebServer httpServer(HTTP_SERVER_PORT);

    HttpServer::HttpServer() {

    }

    void HttpServer::begin() {
        httpServer.on("/start.html", HTTP_GET, []() {
            httpServer.send(200, "text/plain", "Hello World!");
        });
        httpServer.on("/description.xml", HTTP_GET, []() {
            SSDP.schema(httpServer.client());
        });
        httpServer.addHandler(this);
        //http_rest_server.on("/api/HomeAutomation.X10/RF/Control.SendRaw", HTTP_GET, httpApi_X10SendRaw);
        //http_rest_server.on("/leds", HTTP_POST, post_put_leds);
        //http_rest_server.on("/leds", HTTP_PUT, post_put_leds);

        httpServer.begin();
        Logger::info("|  HTTP service started");

        SSDP.setSchemaURL("description.xml");
        SSDP.setHTTPPort(80);
        SSDP.setName("HomeGenie-mini V1.0");
        SSDP.setSerialNumber("ABC0123456789");
        SSDP.setURL("start.html");
        SSDP.setModelName("HomeGenie-mini 2019");
        SSDP.setModelNumber("2134567890");
        SSDP.setModelURL("http://homegenie.it");
        SSDP.setManufacturer("G-Labs");
        SSDP.setManufacturerURL("https://glabs.it");
        SSDP.begin();
        Logger::info("|  SSDP service started");
    }

    void HttpServer::loop() {
        httpServer.handleClient();
    }

    // BEGIN RequestHandler interface methods
    bool HttpServer::canHandle(HTTPMethod method, String uri) {
        return false;
    }
    bool HttpServer::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        return false;
    }

    void HttpServer::addHandler(RequestHandler* handler) {
        httpServer.addHandler(handler);

    }

    // END RequestHandler interface methods

}
