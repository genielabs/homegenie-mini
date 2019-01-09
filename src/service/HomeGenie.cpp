//
// Created by gene on 09/01/19.
//

#include "HomeGenie.h"

namespace Service {

    HomeGenie::HomeGenie() {
        httpServer = new HttpServer();
        ioManager = new IO::IOManager();
    }

    void HomeGenie::begin() {
        httpServer->addHandler(this);
        httpServer->begin();
        ioManager->begin();
    }

    void HomeGenie::loop() {
        httpServer->loop();
    }

    IO::IOManager* HomeGenie::getIOManager() {
        return ioManager;
    }


    // BEGIN RequestHandler interface methods
    bool HomeGenie::canHandle(HTTPMethod method, String uri) {
        return uri != NULL && uri.startsWith("/api/HomeAutomation.X10/");
    }
    bool HomeGenie::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        String res = R"({ "ResponseText": "ERROR" })";
        if (requestUri.startsWith("/api/HomeAutomation.X10/RF/Control.SendRaw/")) {
            String rawBytes = requestUri.substring((uint) requestUri.lastIndexOf('/') + 1);
            uint8_t data[rawBytes.length() / 2]; getBytes(rawBytes, data);
            // Disable Receiver callbacks during transmission to prevent echo
            getIOManager()->getX10Receiver()->disable();
            getIOManager()->getX10Transmitter()->sendCommand(data, sizeof(data));
            getIOManager()->getX10Receiver()->enable();
            res = R"({ "ResponseText": "OK" })";
        }
        server.send(200, "application/json", res);
        return true;
    }

    void HomeGenie::getBytes(const String &rawBytes, uint8_t *data) {
        uint len = rawBytes.length();
        char msg[len+1]; rawBytes.toCharArray(msg, len+1, 0);
        char tmp[3] = "00";
        len = (len / 2);
        for (uint i = 0; i < len; i++)
        {
            tmp[0] = msg[i * 2];
            tmp[1] = msg[(i * 2) + 1];
            data[i] = strtol(tmp, NULL, 16);
        }
    }

}
