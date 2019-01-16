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
 * - 2019-12-01 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_DS18B20_H
#define HOMEGENIE_MINI_DS18B20_H

#include <Arduino.h>
#include <OneWire.h>
#include <Task.h>

#include "io/Logger.h"

#define DS18B20_LOG_PREFIX              "@IO::Env::DS18B10"
#define DS18B20_READ_ERROR -1000

namespace IO { namespace Env {

    class DS18B20 : Task {
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

#endif //HOMEGENIE_MINI_DS18B20_H
