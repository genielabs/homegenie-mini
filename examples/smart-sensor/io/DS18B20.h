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
 * - 2019-01-12 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_DS18B20_H
#define HOMEGENIE_MINI_DS18B20_H

#include <HomeGenie.h>
#include <OneWire.h>

#include "../configuration.h"

#define DS18B20_NS_PREFIX                      "IO::Env::DS18B10"
#define DS18B20_SAMPLING_RATE           60000L
#define DS18B20_READ_ERROR              -1000
// TODO: maybe put this as a configurable parameter through API
#define DS18B20_MEASURE_OFFSET          (float_t)-2.00

namespace IO { namespace Env {

    using namespace Service;

    class DS18B20 : Task, public IIOEventSender {
    public:
        DS18B20() {
            setLoopInterval(DS18B20_SAMPLING_RATE);
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            auto temperature = new ModuleParameter(IOEventPaths::Sensor_Temperature);
            m->properties.add(temperature);
        }
        void begin() override;
        void loop() override;
        void setInputPin(uint8_t);
        void setSamplingRate(uint32_t);
        float_t getTemperature();
    private:
        String domain = IOEventDomains::HomeAutomation_HomeGenie;
        String address = CONFIG_BUILTIN_MODULE_ADDRESS;
        // Default I/O pin number is D3 (0)
        uint8_t inputPin = CONFIG_DS18B20_DataPin;
        // Temperature chip I/O
        OneWire *ds;
        // Current temperature
        float_t currentTemperature;

    };

}}

#endif //HOMEGENIE_MINI_DS18B20_H
