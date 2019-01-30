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
 * - 2019-01-28 Initial release
 *
 */

#include "HomeGenieHandler.h"
#include "X10Handler.h"

bool Service::API::HomeGenieHandler::canHandleDomain(String &domain) {
    return domain == IO::IOEventDomains::HomeAutomation_HomeGenie;
}

bool Service::API::HomeGenieHandler::handleRequest(Service::HomeGenie &homeGenie, Service::APIRequest *request, ESP8266WebServer &server) {
    /*
    if (request->Address.equals("Light") && request->Command.equals("Sensor.GetValue")) {
        char response[1024];
        sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getLightSensor().getLightLevel() / 10.24F);
        request->Response = String(response);
    } else if (request->Address.equals("Temperature") && request->Command.equals("Sensor.GetValue")) {
        char response[1024];
        sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getTemperatureSensor().getTemperature());
        request->Response = String(response);
    } else return false;
    */
    if (request->Address == "Config") {
        if (request->Command == "Modules.List") {
            // HG Mini multi-sensor module
            // TODO: move to getModuleListJSON(ModuleListOutputCallback callback) method like getModuleListJSON in X10Handler.cpp
            auto localModule = getBuiltinModule(homeGenie);
            int msz = homeGenie.writeX10ModuleListJSON(NULL);

            size_t contentLength = (msz+localModule.length()+4);
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "0");
            server.setContentLength(contentLength);
            server.send(200, "application/json; charset=utf-8", "");
            server.sendContent("[\n");
            server.sendContent(localModule);
            homeGenie.writeX10ModuleListJSON(&server);
            server.sendContent("\n]");

            server.client().flush();
            delay(10);
            return true;
        } else if (request->Command == "Modules.Get") {
            String domain = request->getOption(0);
            String address = request->getOption(1);
            if (domain == IOEventDomains::HomeAutomation_X10) {
                // TODO: ...
            } else if (domain == IOEventDomains::HomeAutomation_HomeGenie && address == BUILTIN_MODULE_ADDRESS) {
                request->Response = getBuiltinModule(homeGenie);
                return true;
            }
            return false;
        } else if (request->Command == "Groups.List") {
            // TODO: improve this...
            String list = R"([{"Name":"Dashboard","Modules":[{"Address":"mini","Domain":"HomeAutomation.HomeGenie"}]},)";
            list += R"({"Name":"X10 Modules", "Modules":[)";
            for (int h = 0; h < 16; h++) {
                for (int m = 0; m < 16; m++) {
                    list += R"({"Address":")" + String((char)('A'+h))+String(m + 1) + R"(","Domain":"HomeAutomation.X10"})";
                    if (!(m == 15 && h == 15)) list += ",";
                }
            }
            list += R"(]}])";
            request->Response = list;
            return true;
        }
    }
    return false;
}

bool Service::API::HomeGenieHandler::handleEvent(Service::HomeGenie &homeGenie, IO::IIOEventSender *sender,
                                                 const unsigned char *eventPath, void *eventData,
                                                 IO::IOEventDataType dataType) {

    auto domain = String((char*)sender->getDomain());
    auto address = String((char*)sender->getAddress());
    auto event = String((char*)eventPath);

    // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
    QueuedMessage m = QueuedMessage(domain, address, event, "");
    // Data type handling
    switch (dataType) {
        case SensorLight:
            m.value = String(*(uint16_t *)eventData);
            break;
        case SensorTemperature:
            m.value = String(*(float_t *)eventData);
            break;
        case UnsignedNumber:
            m.value = String(*(uint32_t *)eventData);
            break;
        case Number:
            m.value = String(*(int32_t *)eventData);
            break;
        default:
            m.value = String(*(int32_t *)eventData);
    }
    homeGenie.getEventRouter().signalEvent(m);

    return false;
}

String Service::API::HomeGenieHandler::getBuiltinModule(Service::HomeGenie &homeGenie) {
    auto currentTime = NetManager::getTimeClient().getFormattedDate();
    auto lightSensor = homeGenie.getIOManager().getLightSensor();
    auto temperatureSensor = homeGenie.getIOManager().getTemperatureSensor();
    auto paramLuminance = HomeGenie::createModuleParameter("Sensor.Luminance", String(lightSensor.getLightLevel()).c_str(), currentTime.c_str());
    auto paramTemperature = HomeGenie::createModuleParameter("Sensor.Temperature", String(temperatureSensor.getTemperature()).c_str(), currentTime.c_str());
    return HomeGenie::createModule("HomeAutomation.HomeGenie", BUILTIN_MODULE_ADDRESS,
                                                 "HG-Mini", "HomeGenie Mini node", "Sensor",
                                                 (paramLuminance+","+paramTemperature).c_str());
}
