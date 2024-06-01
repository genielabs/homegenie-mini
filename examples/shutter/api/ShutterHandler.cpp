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
 * - 2024-01-06 Initial release
 *
 */

#include "ShutterHandler.h"

namespace Service { namespace API {

    ShutterHandler::ShutterHandler(ShutterControl* shutterControl) {
        this->shutterControl = shutterControl;

        auto domain = IO::IOEventDomains::Automation_Components;
        // Shutter module
        shutterModule = new Module();
        shutterModule->domain = domain;
        shutterModule->address = SERVO_MODULE_ADDRESS;
        shutterModule->type = "Shutter";
        shutterModule->name = "Shutter 1";
        // add properties
        shutterLevel = new ModuleParameter(IOEventPaths::Status_Level);
        shutterModule->properties.add(shutterLevel);

        shutterControl->setModule(shutterModule);

        moduleList.add(shutterModule);
    }

    void ShutterHandler::init() {

    }


    bool ShutterHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {

        if (command->Domain == (IOEventDomains::Automation_Components)
            && command->Address == SERVO_MODULE_ADDRESS) {

            if (command->Command == "Control.Level") {

                float level = command->OptionsString.toFloat();

                shutterControl->setLevel(level);

                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == "Control.Close" || command->Command == "Control.Off") {

                shutterControl->close();

                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == "Control.Open" || command->Command == "Control.On") {

                shutterControl->open();

                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == "Shutter.Speed") {

                shutterControl->setSpeed(command->OptionsString.toFloat());

                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            } else if (command->Command == "Shutter.Calibrate") {

                // TODO: ...

            } else {

                return false;

            }
            return true;
        }

        return false;
    }

    bool ShutterHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::Automation_Components);
    }

    bool ShutterHandler::handleEvent(IIOEventSender *sender,
                                     const char* domain, const char* address,
                                     const char *eventPath, void *eventData, IOEventDataType dataType) {
        auto module = getModule(domain, address);
        if (module) {
            auto event = String((char *) eventPath);
            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = QueuedMessage(domain, address, event.c_str(), "",
                                   nullptr, IOEventDataType::Undefined);
            // Data type handling
            switch (dataType) {
                case Number:
                    m.value = String(*(int32_t *) eventData);
                    break;
                case Float:
                    m.value = String(*(float *) eventData);
                    break;
                case Text:
                    m.value = String(*(String *) eventData);
                    break;
                default:
                    m.value = String(*(int32_t *) eventData);
            }
            module->setProperty(event, m.value,
                                nullptr, IOEventDataType::Undefined);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }
        return false;
    }

    Module* ShutterHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* ShutterHandler::getModuleList() {
        return &moduleList;
    }

}}
