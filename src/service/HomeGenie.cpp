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
#include "ApiRequest.h"

namespace Service {

    HomeGenie::HomeGenie() {
    }

    void HomeGenie::begin() {
        netManager.begin();
        netManager.getHttpServer().addHandler(this);
        ioManager.begin();
    }

    void HomeGenie::loop() {
        Logger::verbose("  > Service::HomeGenie::loop() >> BEGIN");

        // HomeGenie-Mini Serial CLI
        if(Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            auto apiCommand = ApiRequest::parse(cmd);
            if (apiCommand.Prefix.equals("api")) {
                if (api(&apiCommand)) {
                    Logger::info("=%s", apiCommand.Response.c_str());
                } else {
                    Logger::warn("=%s", apiCommand.Response.c_str());
                }
            }
        }

        Logger::verbose("  > Service::HomeGenie::loop() << END");
    }

    IOManager& HomeGenie::getIOManager() {
        return ioManager;
    }

    // BEGIN RequestHandler interface methods
    bool HomeGenie::canHandle(HTTPMethod method, String uri) {
        return uri != NULL && uri.startsWith("/api/");
    }
    bool HomeGenie::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        auto command = ApiRequest::parse(requestUri);
        if (api(&command)) {
            server.send(200, "application/json", command.Response);
        } else {
            server.send(400, "application/json", command.Response);
        }
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

    bool HomeGenie::api(ApiRequest *command) {
        if (command->Domain.equals("HomeAutomation.X10")
            && command->Address.equals("RF")
            && command->Command.equals("Control.SendRaw")
        ) {

            uint8_t data[command->OptionsString.length() / 2]; getBytes(command->OptionsString, data);
            // Disable RfReceiver callbacks during transmission to prevent echo
            getIOManager().getX10Receiver().disable();
            getIOManager().getX10Transmitter().sendCommand(data, sizeof(data));
            getIOManager().getX10Receiver().enable();
            command->Response = R"({ "ResponseText": "OK" })";

        } else if (command->Domain.equals("HomeAutomation.Env")) {

            if (command->Address.equals("Light") && command->Command.equals("Sensor.GetValue")) {
                char response[1024];
                sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getLightSensor().getLightLevel() / 10.24F);
                command->Response = String(response);
            } else if (command->Address.equals("Temperature") && command->Command.equals("Sensor.GetValue")) {
                char response[1024];
                sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getTemperatureSensor().getTemperature());
                command->Response = String(response);
            } else return false;

        } else return false;
        return true;
    }

}
