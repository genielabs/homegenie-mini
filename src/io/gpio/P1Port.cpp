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
 * - 2019-02-03 Initial release
 *
 */

#include "P1Port.h"

namespace IO { namespace GPIO {

    static uint8_t IOPin[] = { D5, D6, D7, D8 };

    void P1Port::setOutput(uint8_t pinNumber, uint8_t value) {
        auto pin = IOPin[pinNumber-1];
        float level = (value/P1PORT_GPIO_LEVEL_MAX);
        // address is a private member inherited from `IOEventSender` superclass
        address = (const uint8_t*)(String("D")+pinNumber).c_str();
        pinMode(pin, OUTPUT);
        if (value == P1PORT_GPIO_LEVEL_MAX) {
            digitalWrite(pin, HIGH);
        } else if (value == P1PORT_GPIO_LEVEL_MIN) {
            digitalWrite(pin, LOW);
        } else if (value > P1PORT_GPIO_LEVEL_MIN && value < P1PORT_GPIO_LEVEL_MAX) {
            analogWrite(pin, lround((PWMRANGE/P1PORT_GPIO_LEVEL_MAX)*value));
        }
        Logger::info("@%s [%s %d]", P1PORT_NS_PREFIX, (IOEventPaths::Status_Level), String(level).c_str());
        sendEvent((const uint8_t*)(IOEventPaths::Sensor_Luminance), &level, Float);
    }
}}
