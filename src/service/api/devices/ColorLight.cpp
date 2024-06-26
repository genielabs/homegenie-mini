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

#include "ColorLight.h"

namespace Service { namespace API { namespace devices {

    ColorLight::ColorLight(const char* domain, const char* address, const char* name): Dimmer(domain, address, name) {
        module->type = "Color";

        // add properties
        auto propStatusColorHsb = new ModuleParameter(IOEventPaths::Status_ColorHsb);
        propStatusColorHsb->setValue("0,0,1,.4");
        module->properties.add(propStatusColorHsb);

        onSetLevel([this](float l) {
            color.setColor(color.getHue(), color.getSaturation(), l, defaultTransitionMs);
        });
    }

    void ColorLight::loop() {
        Dimmer::loop(); // parent

        if (color.isAnimating()) {
            if (setColorCallback != nullptr) {
                setColorCallback(color);
            }
        }

    }

    bool ColorLight::handleRequest(APIRequest* command, ResponseCallback* responseCallback) {
        if (Dimmer::handleRequest(command, responseCallback)) return true;

        auto m = getModule(command->Domain.c_str(), command->Address.c_str());
        if (m != nullptr && command->Command == ControlApi::Control_ColorHsb) {

            auto hsvString = command->getOption(0);

            float o[4]; int oi = 0;
            int ci;
            do {
                ci = hsvString.indexOf(",");
                if (ci <= 0) {
                    o[oi] = hsvString.toFloat();
                    break;
                }
                o[oi++] = hsvString.substring(0, ci).toFloat();
                hsvString = hsvString.substring(ci + 1);
            } while (oi < 4);

            color.setColor(o[0], o[1], o[2], o[3]*1000);

            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto eventsDisable = module->getProperty(IOEventPaths::Events_Disabled);
            if (eventsDisable == nullptr || eventsDisable->value == nullptr || eventsDisable->value != "1") {
                // color
                auto eventValue = command->getOption(0);
                auto msg = QueuedMessage(m, IOEventPaths::Status_ColorHsb, eventValue, nullptr, IOEventDataType::Undefined);
                m->setProperty(IOEventPaths::Status_ColorHsb, eventValue, nullptr, IOEventDataType::Undefined);
                HomeGenie::getInstance()->getEventRouter().signalEvent(msg);
                // level
                auto levelValue = String(o[2]); // TODO: use sprintf %.6f
                auto msg2 = QueuedMessage(m, IOEventPaths::Status_Level, levelValue, nullptr, IOEventDataType::Undefined);
                m->setProperty(IOEventPaths::Status_Level, levelValue, nullptr, IOEventDataType::Undefined);
                HomeGenie::getInstance()->getEventRouter().signalEvent(msg2);
            }

            if (o[2] > 0) {
                Switch::status = SWITCH_STATUS_ON;
                Dimmer::onLevel = Switch::onLevel = o[2];
            } else {
                Switch::status = SWITCH_STATUS_OFF;
            }

            responseCallback->writeAll(ApiHandlerResponseText::OK);
            return true;

        }
        // not handled
        return false;
    }

}}}