//
// Created by gene on 11/01/19.
//

#ifndef HOMEGENIE_MINI_DS18B10_H
#define HOMEGENIE_MINI_DS18B10_H

#include <Arduino.h>
#include <OneWire.h>

#include "io/Logger.h"

#define DS18B10_READ_ERROR -1000

namespace IO { namespace Env {

    class DS18B10 {
    public:
        void begin();
        void loop();
        void setInputPin(int pinNumber);
        void setSamplingRate(const unsigned int samplingRate);
        float getTemperature();
    private:
        // default sampling rate of 5 minutes
        unsigned long samplingRate = 5 * 60 * 1000UL;
        // default pin number is 2
        int pinNumber = 2;
        //Temperature chip i/o
        OneWire *ds;
    };

}}

#endif //HOMEGENIE_MINI_DS18B10_H
