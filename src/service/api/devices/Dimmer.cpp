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

#include "Dimmer.h"

namespace Service { namespace API { namespace devices {

    Dimmer::Dimmer(const char* domain, const char* address, const char* name): Switch(domain, address, name) {
        setLoopInterval(10); // fixed transition frequency
        module->type = "Dimmer";
        onSetStatus([this](SwitchStatus status) {
            level.setLevel(status == SWITCH_STATUS_ON ? 1 : 0, defaultTransitionMs);
        });
    }

    void Dimmer::loop() {

        if (level.isAnimating) {
            if (setLevelCallback != nullptr) {
                setLevelCallback(level.getLevel());
            }
        }

    }

    bool Dimmer::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (Switch::handleRequest(command, responseCallback)) return true;

        auto m = getModule(command->Domain.c_str(), command->Address.c_str());
        if (m != nullptr && command->Command == ControlApi::Control_Level) {

            auto l = command->getOption(0).toFloat() / 100.0F; // 0.00 - 1.00  0 = OFF, 1.00 = MAX
            auto transition = command->getOption(1).isEmpty() ? defaultTransitionMs : command->getOption(1).toFloat(); // ms

            level.setLevel(l, transition);

            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto eventPath = IOEventPaths::Status_Level;
            auto eventValue = String(l);
            auto msg = QueuedMessage(m, eventPath, eventValue, &l, IOEventDataType::Float);
            m->setProperty(eventPath, eventValue, &l, IOEventDataType::Float);
            HomeGenie::getInstance()->getEventRouter().signalEvent(msg);

            if (l > 0) {
                Switch::status = SWITCH_STATUS_ON;
                Switch::onLevel = l;
            } else {
                Switch::status = SWITCH_STATUS_OFF;
            }

            responseCallback->writeAll(R"({ "ResponseText": "OK" })");
            return true;

        }
        // not handled
        return false;
    }

}}}