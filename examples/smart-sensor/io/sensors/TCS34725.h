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
 * - 2024-10-17 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_TCS34725_H
#define HOMEGENIE_MINI_TCS34725_H

#include <Adafruit_TCS34725.h>

#include <HomeGenie.h>

#define COLOR_SENSOR_NS_PREFIX           "IO::Sensors::ColorSensor"

namespace IO { namespace Sensors {

    using namespace Service;

    class TCS34725 : Task, public IIOEventSender {
    public:
        explicit TCS34725(uint8_t sdaPin, uint8_t sclPin) {
            setLoopInterval(10000);
            this->sdaPin = sdaPin;
            this->sclPin = sclPin;
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            m->setProperty(IOEventPaths::Sensor_Luminance, "0");
        }
        void begin() override;
        void loop() override;
    private:
        Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
        uint8_t sdaPin = 4;
        uint8_t sclPin = 5;
        uint16_t lightLevel = 0;
        bool initialized = false;
    };

}} // IO::Sensors


#endif //HOMEGENIE_MINI_TCS34725_H
