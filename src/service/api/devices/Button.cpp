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

#include "Button.h"

namespace Service { namespace API { namespace devices {

    Button::Button(const char* domain, const char* address, const char* name, uint8_t pin, PullResistor resistor) {
        module = new Module();
        module->domain = domain;
        module->address = address;
        module->type = ModuleApi::ModuleType::Switch;
        module->name = name;
        // set properties
        module->setProperty(IOEventPaths::Status_Level, "0");
        moduleList.add(module);

        pinNumber = pin;
        pinMode(pin, resistor);
    }

    void Button::init() {
        setLoopInterval(50); // ~50ms response time
    }
    void Button::loop() {
        bool pressed = digitalRead(pinNumber) == LOW;
        bool changed = false;

        if (pressed && status != BUTTON_STATUS_PRESSED) {
            status = BUTTON_STATUS_PRESSED;
            pressedTs = millis();
            changed = true;
        } else if (!pressed && status == BUTTON_STATUS_PRESSED) {
            status = BUTTON_STATUS_NORMAL;
            changed = true;
        }

        if (changed) {

            if (setStatusCallback != nullptr) {
                setStatusCallback(status);
            }
            if (status == BUTTON_STATUS_NORMAL && millis() - pressedTs < 500) {
                onGestureCallback(ButtonGesture::BUTTON_GESTURE_CLICK);
            }

            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto eventValue = String(status);
            auto msg = QueuedMessage(module, IOEventPaths::Status_Level, eventValue, &status, IOEventDataType::Number);
            module->setProperty(IOEventPaths::Status_Level, eventValue, &status, IOEventDataType::Number);
            HomeGenie::getInstance()->getEventRouter().signalEvent(msg);

        } else if (status == BUTTON_STATUS_PRESSED && millis() - pressedTs > 500 && pressedTs != -1) {

            pressedTs = -1;
            onGestureCallback(ButtonGesture::BUTTON_GESTURE_LONG_PRESS);

        }
    }

    bool Button::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        auto m = getModule(command->Domain.c_str(), command->Address.c_str());
        if (m != nullptr) {

            // TODO: implement device API
            // return true; // <-- if API call was handled

        }
        // not handled
        return false;
    }

    bool Button::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool Button::handleEvent(IIOEventSender *sender,
                             const char* domain, const char* address,
                             const char *eventPath, void *eventData, IOEventDataType dataType) {
        return false;
    }

    Module* Button::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* m = moduleList.get(i);
            if (m->domain.equals(domain) && m->address.equals(address))
                return m;
        }
        return nullptr;
    }

    LinkedList<Module*>* Button::getModuleList() {
        return &moduleList;
    }

}}}
