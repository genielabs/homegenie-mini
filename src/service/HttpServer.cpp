//
// Created by gene on 09/01/19.
//

#include "HttpServer.h"

namespace Service {

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
        Serial.println("HTTP REST Server Started");

        Serial.printf("Starting SSDP...\n");
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
