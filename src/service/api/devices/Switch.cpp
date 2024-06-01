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
 */

#include "Switch.h"

namespace Service { namespace API { namespace devices {

    Switch::Switch(const char* domain, const char* address, const char* name) {
        module = new Module();
        module->domain = domain;
        module->address = address;
        module->type = "Switch";
        module->name = name;
        // add properties
        auto propStatusLevel = new ModuleParameter(IOEventPaths::Status_Level);
        propStatusLevel->value = "0";
        module->properties.add(propStatusLevel);

        moduleList.add(module);
    }

    void Switch::init() {
    }

    bool Switch::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        auto m = getModule(command->Domain.c_str(), command->Address.c_str());
        if (m != nullptr) {

            auto eventPath = IOEventPaths::Status_Level;
            SwitchStatus s = SWITCH_STATUS_NOT_SET;
            if (command->Command == ControlApi::Control_On) {
                s = SWITCH_STATUS_ON;
            } else if (command->Command == ControlApi::Control_Off) {
                s = SWITCH_STATUS_OFF;
            } else if (command->Command == ControlApi::Control_Toggle) {
                s = status == SWITCH_STATUS_ON ? SWITCH_STATUS_OFF : SWITCH_STATUS_ON;
            }

            if (s != SWITCH_STATUS_NOT_SET) {
                status = s;

                if (setStatusCallback != nullptr) {
                    setStatusCallback(status);
                }

                // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
                auto eventsDisable = module->getProperty(IOEventPaths::Events_Disabled);
                if (eventsDisable == nullptr || eventsDisable->value == nullptr || eventsDisable->value != "1") {
                    float l = status == SWITCH_STATUS_ON ? onLevel : 0;
                    auto eventValue = String(l);
                    auto msg = QueuedMessage(m, eventPath, eventValue, &l, IOEventDataType::Float);
                    m->setProperty(eventPath, eventValue, &l, IOEventDataType::Float);
                    HomeGenie::getInstance()->getEventRouter().signalEvent(msg);
                }

                responseCallback->writeAll(ApiHandlerResponseText::OK);

                return true;
            }

        }
        // not handled
        return false;
    }

    bool Switch::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool Switch::handleEvent(IIOEventSender *sender,
                             const char* domain, const char* address,
                             const char *eventPath, void *eventData, IOEventDataType dataType) {
        return false;
    }

    Module* Switch::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* m = moduleList.get(i);
            if (m->domain.equals(domain) && m->address.equals(address))
                return m;
        }
        return nullptr;
    }

    LinkedList<Module*>* Switch::getModuleList() {
        return &moduleList;
    }

}}}
