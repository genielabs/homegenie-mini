//
// Created by gene on 19/01/19.
//

#ifndef HOMEGENIE_MINI_IIOEVENT_H
#define HOMEGENIE_MINI_IIOEVENT_H

#include <Arduino.h>

#define IOEVENT_DOMAIN__HOMEAUTOMATION_X10           "HomeAutomation.X10"

namespace IO {

    class IIOEventSender;

    enum IOEventDataType {
        Undefined = 0,
        Number,
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
        virtual void sendEvent(uint8_t *eventPath, void* eventData, IOEventDataType dataType = Undefined) {
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
