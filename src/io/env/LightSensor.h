//
// Created by gene on 11/01/19.
//

#ifndef HOMEGENIE_MINI_LIGHTSENSOR_H
#define HOMEGENIE_MINI_LIGHTSENSOR_H

#include <io/Logger.h>

namespace IO { namespace Env {

    class LightSensor {
    public:
        void begin();
        void loop();
        void setInputPin(uint8_t number);
        float getLightLevel();
    private:
        // default sampling rate of 1 second
        unsigned long samplingRate = 1000UL;
        uint8_t inputPin = 0;
    };

}}

#endif //HOMEGENIE_MINI_LIGHTSENSOR_H
