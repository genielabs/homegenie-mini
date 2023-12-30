/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
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

    HomeGenieHandler::HomeGenieHandler(GPIOPort* gpioPort) {
        this->gpioPort = gpioPort;
        // HomeGenie Mini module (default module)
        auto miniModule = new Module();
        miniModule->domain = IO::IOEventDomains::HomeAutomation_HomeGenie;
        miniModule->address = CONFIG_BUILTIN_MODULE_ADDRESS;
        miniModule->type = "Sensor";
        miniModule->name = CONFIG_BUILTIN_MODULE_NAME;
        miniModule->description = "HomeGenie Mini node";
        moduleList.add(miniModule);
    }

    void HomeGenieHandler::init() {
        // Add GPIO modules
        // Output GPIO pins
        uint8_t gpio[] = CONFIG_GPIO_OUT;
        uint8_t gpio_count = *(&gpio + 1) - gpio;
        for (int m = 0; m < gpio_count; m++) {
            auto address = String(gpio[m]);
            auto module = new Module();
            module->domain = IO::IOEventDomains::HomeAutomation_HomeGenie;
            module->address = address;
            module->type = "Dimmer";
            module->name = "GPIO " + address;
            module->description = "";

            // init pins with current level
            float level = GPIOPort::loadLevel(gpio[m]);
            level > 0 ? gpioPort->on(gpio[m]) : gpioPort->off(gpio[m]);
            auto propLevel = new ModuleParameter(IOEventPaths::Status_Level, String(level));
            module->properties.add(propLevel);

            moduleList.add(module);
        }

        // TODO: implement Input GPIO pins as well (CONFIG_GPIO_IN)

    }

    bool HomeGenieHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool HomeGenieHandler::handleRequest(Service::APIRequest *request, WebServer &server) {
        auto homeGenie = HomeGenie::getInstance();
        if (request->Address == "Config") {
            if (request->Command == "Modules.List") {
                auto contentLength = (size_t)homeGenie->writeModuleListJSON(nullptr);
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie->writeModuleListJSON(&server);
                //server.client().flush();
                return true;
            } else if (request->Command == "Modules.Get") {
                String domain = request->getOption(0);
                String address = request->getOption(1);
                auto contentLength = (size_t)homeGenie->writeModuleJSON(nullptr, &domain, &address);
                if (contentLength == 0) return false;
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie->writeModuleJSON(&server, &domain, &address);
                return true;
            } else if (request->Command == "Groups.List") {
                auto contentLength = (size_t ) homeGenie->writeGroupListJSON(nullptr);
                server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
                server.sendHeader("Pragma", "no-cache");
                server.sendHeader("Expires", "0");
                server.setContentLength(contentLength);
                server.send(200, "application/json; charset=utf-8", "");
                homeGenie->writeGroupListJSON(&server);
                //server.client().flush();
                return true;
            } else if (request->Command == "WebSocket.GetToken") {

                // TODO: implement random token with expiration (like in HG server) for websocket client verification

                request->Response = R"({ "ResponseValue": "e046f885-1d51-4dd2-b952-38e7134a9c0f" })";
                return true;
            }
        } else {
            uint8_t gpio[] = CONFIG_GPIO_OUT;
            uint8_t pinNumber = request->Address.toInt();
            bool validPin = std::find(std::begin(gpio), std::end(gpio), pinNumber) != std::end(gpio);
            if (validPin) {

                // TODO: move to utility class -- lookup module by domain/address

                Module* module;
                for (int m = 0; m < moduleList.size(); m++) {
                    module = moduleList[m];
                    if (module->domain.equals(request->Domain) && module->address.equals(request->Address)) {
                        break;
                    }
                    module = nullptr;
                }

                if (module) {

                    auto levelProperty = module->getProperty(IOEventPaths::Status_Level);
                    if (levelProperty->value.toFloat() > 0) {
                        GPIOPort::saveLastOnLevel(pinNumber, levelProperty->value.toFloat());
                    }

                    float level = 0;
                    if (request->Command == "Control.On") {
                        level = gpioPort->on(pinNumber);
                    } else if (request->Command == "Control.Off") {
                        level = gpioPort->off(pinNumber);
                    } else if (request->Command == "Control.Level") {
                        level = gpioPort->level(pinNumber, (uint8_t)request->getOption(0).toFloat());
                    } else if (request->Command == "Control.Toggle") {
                        if (levelProperty->value.toFloat() == 0) {
                            level = gpioPort->on(pinNumber);
                        } else {
                            level = gpioPort->off(pinNumber);
                        }
                    }

                    levelProperty->setValue(String(level).c_str());
                    GPIOPort::saveLevel(pinNumber, levelProperty->value.toFloat());

                    request->Response = R"({ "ResponseText": "OK" })";
                    return  true;
                }
            }
        }
        return false;
    }

    bool HomeGenieHandler::handleEvent(IO::IIOEventSender *sender,
                                       const char* domain, const char* address,
                                       const unsigned char *eventPath, void *eventData,
                                       IO::IOEventDataType dataType) {
        auto module = getModule(domain, address);
        if (module) {
            auto event = String((char *) eventPath);
            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = QueuedMessage(domain, address, event.c_str(), "");
            // Data type handling
            switch (dataType) {
                case SensorLight:
                    m.value = String(*(uint16_t *) eventData);
                    break;
                case SensorTemperature:
                    m.value = String(*(float_t *) eventData);
                    break;
                case UnsignedNumber:
                    m.value = String(*(uint32_t *) eventData);
                    break;
                case Number:
                    m.value = String(*(int32_t *) eventData);
                    break;
                case Float:
                    m.value = String(*(float *) eventData);
                    break;
                default:
                    m.value = String(*(int32_t *) eventData);
            }
            module->setProperty(event, m.value);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }
        return false;
    }

    Module* HomeGenieHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* HomeGenieHandler::getModuleList() {
        return &moduleList;
    }

}}
