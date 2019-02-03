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
 * - 2019-01-19 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_IIOEVENT_H
#define HOMEGENIE_MINI_IIOEVENT_H

#include <Arduino.h>

namespace IO {

    class IIOEventSender;

    enum IOEventDataType {
        Undefined = 0,
        Number,
        Float,
        UnsignedNumber,
        SensorLight,
        SensorTemperature
    };

// IIOEventReceiver interface
    class IIOEventReceiver {
    public:
        virtual void onIOEvent(IIOEventSender *, const uint8_t *, void *, IOEventDataType dataType = Undefined) = 0; // pure virtual
    };

// IIOEventSender interface
    class IIOEventSender {
    public:
        const uint8_t* getDomain() { return domain; }
        const uint8_t* getAddress() { return address; }
        void setEventReceiver(IIOEventReceiver *receiver) {
            eventReceiver = receiver;
        }
        virtual void sendEvent(const uint8_t *eventPath, void *eventData, IOEventDataType dataType) {
            if (eventReceiver != NULL) {
                eventReceiver->onIOEvent(this, eventPath, eventData, dataType);
            }
        };

    protected:
        const uint8_t *domain;
        const uint8_t *address;
        IIOEventReceiver *eventReceiver;
    };

}

#endif //HOMEGENIE_MINI_IIOEVENT_H
