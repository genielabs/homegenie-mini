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
        // set properties
        module->setProperty(IOEventPaths::Status_Level, "0");

        moduleList.add(module);
    }

    void Switch::init() {
    }
    void Switch::loop() {
    }

    bool Switch::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        auto m = getModule(command->Domain.c_str(), command->Address.c_str());
        if (m != nullptr) {

            bool handled = true;
            if (command->Command == ControlApi::Control_On) {
                on();
            } else if (command->Command == ControlApi::Control_Off) {
                off();
            } else if (command->Command == ControlApi::Control_Toggle) {
                toggle();
            } else {
                handled = false;
            }

            if (handled) {
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

    void Switch::on() {
        setStatus(SWITCH_STATUS_ON);
    }

    void Switch::off() {
        setStatus(SWITCH_STATUS_OFF);
    }

    void Switch::toggle() {
        setStatus(status == SWITCH_STATUS_ON ? SWITCH_STATUS_OFF : SWITCH_STATUS_ON);
    }

    void Switch::setStatus(SwitchStatus s) {

        status = s;

        if (setStatusCallback != nullptr) {
            setStatusCallback(status);
        }

        // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
        auto eventPath = IOEventPaths::Status_Level;
        auto eventsDisable = module->getProperty(IOEventPaths::Events_Disabled);
        if (eventsDisable == nullptr || eventsDisable->value == nullptr || eventsDisable->value != "1") {
            float l = status == SWITCH_STATUS_ON ? onLevel : 0;
            auto eventValue = String(l);
            auto msg = QueuedMessage(module, eventPath, eventValue, &l, IOEventDataType::Float);
            module->setProperty(eventPath, eventValue, &l, IOEventDataType::Float);
            HomeGenie::getInstance()->getEventRouter().signalEvent(msg);
        }

    }
}}}
