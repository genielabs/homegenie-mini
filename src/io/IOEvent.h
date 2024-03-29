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
 * - 2019-01-19 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_IIOEVENT_H
#define HOMEGENIE_MINI_IIOEVENT_H

#include "Config.h"

#include "data/Module.h"
#include "IOEventData.h"

namespace IO {

    using namespace Data;

    class IIOEventSender;

    // IIOEventReceiver interface
    class IIOEventReceiver {
    public:
        virtual void onIOEvent(IIOEventSender *, const char *, const char *, const uint8_t *, void *, IOEventDataType dataType) = 0; // pure virtual
    };

    // IIOEventSender interface
    class IIOEventSender {
    public:
        virtual void begin() = 0;
        void setEventReceiver(IIOEventReceiver *receiver) {
            eventReceiver = receiver;
        }

        virtual void setModule(Module* m) {
            module = m;
        }
        virtual void sendEvent(const char *domain, const char *address, const uint8_t *eventPath, void *eventData, IOEventDataType dataType) {
            if (eventReceiver != nullptr) {
                eventReceiver->onIOEvent(this, domain, address, eventPath, eventData, dataType);
                lastEventMs = millis();
            }
        };
        virtual void sendEvent(const uint8_t *eventPath, void *eventData, IOEventDataType dataType) {
            if (eventReceiver != nullptr && module != nullptr) {
                eventReceiver->onIOEvent(this, module->domain.c_str(), module->address.c_str(), eventPath, eventData, dataType);
                lastEventMs = millis();
            }
        };

    protected:
        IIOEventReceiver *eventReceiver = nullptr;
        unsigned long lastEventMs = 0;
        const Module* module = nullptr;
    };

}

#endif //HOMEGENIE_MINI_IIOEVENT_H
