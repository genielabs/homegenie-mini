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

    bool HomeGenieHandler::handleRequest(Service::APIRequest *request, ResponseCallback* responseCallback) {
        auto homeGenie = HomeGenie::getInstance();
        if (request->Address == F("Config")) {
            if (request->Command == ConfigApi::Groups_List) {
                responseCallback->beginGetLength();
                homeGenie->writeGroupListJSON(responseCallback);
                responseCallback->endGetLength();
                homeGenie->writeGroupListJSON(responseCallback);
                return true;
            } else if (request->Command == ConfigApi::Modules_List) {
                responseCallback->beginGetLength();
                homeGenie->writeModuleListJSON(responseCallback);
                responseCallback->endGetLength();
                homeGenie->writeModuleListJSON(responseCallback);
                return true;
            } else if (request->Command == ConfigApi::Modules_Get) {
                String domain = request->getOption(0);
                String address = request->getOption(1);
                responseCallback->beginGetLength();
                auto contentLength = (size_t)homeGenie->writeModuleJSON(responseCallback, &domain, &address);
                responseCallback->endGetLength();
                if (contentLength == 0) return false;
                homeGenie->writeModuleJSON(responseCallback, &domain, &address);
                return true;
            } else if (request->Command == ConfigApi::Modules_ParameterSet) {
                String domain = request->getOption(0);
                String address = request->getOption(1);
                String propName = request->getOption(2);
                String propValue = WebServer::urlDecode(request->getOption(3));
                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    module->setProperty(propName, propValue, nullptr, IOEventDataType::Undefined);
                    QueuedMessage m;
                    m.domain = domain;
                    m.sender = address;
                    m.event = propName;
                    m.value = propValue;
                    homeGenie->getEventRouter().signalEvent(m);
                    responseCallback->writeAll(R"({ "ResponseText": "OK" })");
                    return true;
                }
            } else if (request->Command == ConfigApi::WebSocket_GetToken) {

                // TODO: implement random token with expiration (like in HG server) for websocket client verification

                responseCallback->writeAll(R"({ "ResponseValue": "e046f885-1d51-4dd2-b952-38e7134a9c0f" })");
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
                    if (request->Command == ControlApi::Control_On) {
                        level = gpioPort->on(pinNumber);
                    } else if (request->Command == ControlApi::Control_Off) {
                        level = gpioPort->off(pinNumber);
                    } else if (request->Command == ControlApi::Control_Level) {
                        level = gpioPort->level(pinNumber, (uint8_t)request->getOption(0).toFloat());
                    } else if (request->Command == ControlApi::Control_Toggle) {
                        if (levelProperty->value.toFloat() == 0) {
                            level = gpioPort->on(pinNumber);
                        } else {
                            level = gpioPort->off(pinNumber);
                        }
                    }

                    levelProperty->setValue(String(level).c_str());
                    GPIOPort::saveLevel(pinNumber, levelProperty->value.toFloat());

                    responseCallback->writeAll(R"({ "ResponseText": "OK" })");
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
            auto m = QueuedMessage(domain, address, event.c_str(), "", eventData, dataType);
            // Data type handling
            switch (dataType) {
                case SensorLight:
                    m.value = String(*(uint16_t *) eventData);
                    break;
                case SensorTemperature:
                    m.value = String(*(float_t *) eventData);
                    break;
                case SensorHumidity:
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
            module->setProperty(event, m.value, eventData, dataType);
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
