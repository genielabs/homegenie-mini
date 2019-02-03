//
// Created by gene on 03/02/19.
//

#ifndef HOMEGENIE_MINI_P1PORT_H
#define HOMEGENIE_MINI_P1PORT_H

#include <Arduino.h>
#include <io/IOEvent.h>
#include <io/IOModule.h>
#include <io/IOEventDomains.h>
#include <io/IOEventPaths.h>
#include <io/Logger.h>

#define P1PORT_NS_PREFIX           "IO::GPIO::P1Port"

#define P1PORT_GPIO_COUNT           4
#define P1PORT_GPIO_LEVEL_MIN       0.0F
#define P1PORT_GPIO_LEVEL_MAX       100.0F

namespace IO { namespace GPIO {

    class P1Port: public IIOEventSender  {
    public:
        P1Port() {
            // IEventSender members
            domain = (const uint8_t*)IOEventDomains::HomeAutomation_HomeGenie;
        }
        void setOutput(uint8_t pinNumber, uint8_t value);
    };

}}


#endif //HOMEGENIE_MINI_P1PORT_H
