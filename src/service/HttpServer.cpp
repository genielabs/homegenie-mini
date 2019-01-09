//
// Created by gene on 09/01/19.
//

#include "HttpServer.h"

namespace Service {

    ESP8266WebServer httpServer(HTTP_SERVER_PORT);

    HttpServer::HttpServer() {

    }

    void HttpServer::begin() {
        httpServer.on("/index.html", HTTP_GET, []() {
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
        SSDP.setName("Philips hue clone");
        SSDP.setSerialNumber("001788102201");
        SSDP.setURL("index.html");
        SSDP.setModelName("Philips hue bridge 2012");
        SSDP.setModelNumber("929000226503");
        SSDP.setModelURL("http://www.meethue.com");
        SSDP.setManufacturer("Royal Philips Electronics");
        SSDP.setManufacturerURL("http://www.philips.com");
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
