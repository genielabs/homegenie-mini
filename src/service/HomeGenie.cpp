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
        ioManager->loop();
        httpServer->loop();
        // TODO: move the following to a separate class or method
        // HomeGenie-Mini Serial CLI
        if(Serial.available() > 0)
        {
            String cmd = Serial.readStringUntil('\n');
            if (cmd.startsWith("X10:send "))
            {
                String hexData = cmd.substring(9);
                uint8_t data[hexData.length() / 2]; getBytes(hexData, data);
                // Disable RfReceiver callbacks during transmission to prevent echo
                getIOManager().getX10Receiver().disable();
                getIOManager().getX10Transmitter().sendCommand(data, sizeof(data));
                getIOManager().getX10Receiver().enable();
            }
        }
    }

    IO::IOManager& HomeGenie::getIOManager() {
        return *ioManager;
    }

    // BEGIN RequestHandler interface methods
    bool HomeGenie::canHandle(HTTPMethod method, String uri) {
        return uri != NULL && uri.startsWith("/api/");
    }
    bool HomeGenie::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        String res = R"({ "ResponseText": "ERROR" })";
        if (requestUri.startsWith("/api/HomeAutomation.X10/RF/Control.SendRaw/")) {
            String rawBytes = requestUri.substring((uint) requestUri.lastIndexOf('/') + 1);
            uint8_t data[rawBytes.length() / 2]; getBytes(rawBytes, data);
            // Disable RfReceiver callbacks during transmission to prevent echo
            getIOManager().getX10Receiver().disable();
            getIOManager().getX10Transmitter().sendCommand(data, sizeof(data));
            getIOManager().getX10Receiver().enable();
            res = R"({ "ResponseText": "OK" })";
        } else if (requestUri.startsWith("/api/HomeAutomation.Env/Light/Sensor.GetValue")) {
            char response[1024];
            sprintf(response, R"({ "ResponseText": "%0.2f" })", ioManager->getLightSensor().getLightLevel() / 10.24F);
            res = String(response);
        } else if (requestUri.startsWith("/api/HomeAutomation.Env/Temperature/Sensor.GetValue")) {
            char response[1024];
            sprintf(response, R"({ "ResponseText": "%0.2f" })", ioManager->getTemperatureSensor().getTemperature());
            res = String(response);
        }
        server.send(200, "application/json", res);
        return true;
    }
    // END RequestHandler interface methods

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
