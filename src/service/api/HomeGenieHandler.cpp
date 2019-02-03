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

namespace Service { namespace API {

    static IOModule moduleList[P1PORT_GPIO_COUNT];

    bool HomeGenieHandler::canHandleDomain(String &domain) {
        return domain == (IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool HomeGenieHandler::handleRequest(Service::HomeGenie &homeGenie, Service::APIRequest *request, ESP8266WebServer &server) {
        if (request->Address.length() == 2 && request->Address.startsWith("D")) {
            uint8_t pinNumber = request->Address.substring(1).toInt()-4;
            if (pinNumber >= 1 && pinNumber <= P1PORT_GPIO_COUNT) {
                QueuedMessage m = QueuedMessage(request->Domain, request->Address, (IOEventPaths::Status_Level), "");
                auto module = &moduleList[pinNumber];
                auto gpioPort = homeGenie.getIOManager().getExpansionPort();
                if (request->Command == "Control.On") {
                    gpioPort.setOutput(pinNumber, P1PORT_GPIO_LEVEL_MAX);
                    module->Level = 1;
                } else if (request->Command == "Control.Off") {
                    gpioPort.setOutput(pinNumber, P1PORT_GPIO_LEVEL_MIN);
                    module->Level = 0;
                } else if (request->Command == "Control.Level") {
                    uint8_t level = (uint8_t)request->getOption(0).toInt();
                    gpioPort.setOutput(pinNumber, level);
                    module->Level = (level/P1PORT_GPIO_LEVEL_MAX);
                } else if (request->Command == "Control.Toggle") {
                    if (module->Level == 0) {
                        gpioPort.setOutput(pinNumber, P1PORT_GPIO_LEVEL_MAX);
                        module->Level = 1;
                    } else {
                        gpioPort.setOutput(pinNumber, P1PORT_GPIO_LEVEL_MIN);
                        module->Level = 0;
                    }
                }
                m.value = String(module->Level);
                homeGenie.getEventRouter().signalEvent(m);
                request->Response = R"({ "ResponseText": "OK" })";
                return  true;
            } else return false;
        } else if (request->Address == "Config") {
            if (request->Command == "Modules.List") {
                // HG Mini multi-sensor module
                auto contentLength = (size_t)homeGenie.writeModuleListJSON(NULL);
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie.writeModuleListJSON(&server);
                //server.client().flush();
                return true;
            } else if (request->Command == "Modules.Get") {
                String domain = request->getOption(0);
                String address = request->getOption(1);
                auto contentLength = (size_t)homeGenie.writeModuleJSON(NULL, domain, address);
                if (contentLength == 0) return false;
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie.writeModuleJSON(&server, domain, address);
                return true;
            } else if (request->Command == "Groups.List") {
                auto contentLength = (size_t ) homeGenie.writeGroupListJSON(NULL);
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie.writeGroupListJSON(&server);
                //server.client().flush();
                return true;
            }
        }
        return false;
    }

    bool HomeGenieHandler::handleEvent(Service::HomeGenie &homeGenie, IO::IIOEventSender *sender,
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
            case Float:
                m.value = String(*(float *)eventData);
                break;
            default:
                m.value = String(*(int32_t *)eventData);
        }
        homeGenie.getEventRouter().signalEvent(m);

        return false;
    }

    void HomeGenieHandler::getModuleJSON(OutputStreamCallback *outputCallback, String &domain, String &address) {
        if (address.length() != 2) return;
        int pinNumber = address.substring(1).toInt()-P1PORT_GPIO_COUNT;
        if (pinNumber >= 1 && pinNumber <= P1PORT_GPIO_COUNT) {
            auto module = &moduleList[pinNumber];
            auto paramLevel = String(module->Level);
            auto deviceType = String("Dimmer"); // TODO: DeviceTypes[module->Type];
            if (module->UpdateTime.startsWith("1970-")) {
                module->UpdateTime = NetManager::getTimeClient().getFormattedDate();
            }
            paramLevel = HomeGenie::createModuleParameter(IOEventPaths::Status_Level, paramLevel.c_str(), module->UpdateTime.c_str());
            auto moduleJSON = HomeGenie::createModule(domain.c_str(), address.c_str(),
                                                      "", "P1 Module", deviceType.c_str(),
                                                      paramLevel.c_str());
            outputCallback->write(moduleJSON);
        }
    }

    void HomeGenieHandler::getModuleListJSON(OutputStreamCallback *outputCallback) {
        auto domain = String((IOEventDomains::HomeAutomation_HomeGenie));
        auto separator = String(",\n");
        // P1 Expansion Port modules
        for (int m = 0; m < P1PORT_GPIO_COUNT; m++) {
            auto address = "D"+String(m+1+P1PORT_GPIO_COUNT);
            if (m != 0) outputCallback->write(separator);
            getModuleJSON(outputCallback, domain, address);
        }
    }

}}
