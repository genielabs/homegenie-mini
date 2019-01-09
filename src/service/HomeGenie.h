//
// Created by gene on 09/01/19.
//

#ifndef HOMEGENIE_MINI_HOMEGENIE_H
#define HOMEGENIE_MINI_HOMEGENIE_H

#include <io/IOManager.h>
#include "HttpServer.h"

#include <detail/RequestHandler.h>

namespace Service {

    using namespace IO;

    class HomeGenie: RequestHandler {
    public:
        HomeGenie();
        void begin();
        void loop();

        bool canHandle(HTTPMethod method, String uri);
        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);

        IOManager& getIOManager();
        //void getHttpServer();
    private:
        IOManager *ioManager;
        HttpServer *httpServer;
        void getBytes(const String &rawBytes, uint8_t *data);
    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H
