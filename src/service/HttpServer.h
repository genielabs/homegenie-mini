//
// Created by gene on 09/01/19.
//

#ifndef HOMEGENIE_MINI_HTTPSERVER_H
#define HOMEGENIE_MINI_HTTPSERVER_H

#include <ESP8266WebServer.h>
#include <ESP8266SSDP.h>
#include <detail/RequestHandler.h>

#include "HttpServer.h"

#define HTTP_SERVER_PORT 80

namespace Service {

    class HttpServer : RequestHandler {
    public:
        HttpServer();
        void begin();
        void loop();
        void addHandler(RequestHandler* handler);
        // RequestHandler interface methods
        bool canHandle(HTTPMethod method, String uri);
        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);
    };

}

#endif //HOMEGENIE_MINI_HTTPSERVER_H
