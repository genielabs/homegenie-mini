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
 * - 2019-02-03 Initial release
 *
 */

#include "GPIOPort.h"

namespace IO { namespace GPIO {

    void GPIOPort::begin() {}

    void GPIOPort::setInput(uint8_t pinNumber, void(*callback)()) {
        pinMode(pinNumber, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(pinNumber), callback, CHANGE);
    }
    void GPIOPort::setOutput(uint8_t pinNumber, uint8_t value) {
        float level = (value/(GPIO_PORT_LEVEL_MAX - GPIO_PORT_LEVEL_MIN));
        // address is a private member inherited from `IOEventSender` superclass
        String address = (String(pinNumber)).c_str();
        pinMode(pinNumber, OUTPUT);
        if (value == GPIO_PORT_LEVEL_MAX) {
            digitalWrite(pinNumber, HIGH);
        } else if (value == GPIO_PORT_LEVEL_MIN) {
            digitalWrite(pinNumber, LOW);
        } else if (value > GPIO_PORT_LEVEL_MIN && value < GPIO_PORT_LEVEL_MAX) {
            analogWrite(pinNumber, value);
        }
        Logger::info("@%s [%s %.2f]", GPIO_PORT_NS_PREFIX, (IOEventPaths::Status_Level), level);
        sendEvent(IO::IOEventDomains::HomeAutomation_HomeGenie, address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &level, Float);
    }


    float GPIOPort::on(uint8_t pinNumber) {
        float savedLevel = loadLastOnLevel(pinNumber);
        savedLevel = savedLevel > 0 ? savedLevel : 1;
        float gpioLevel = GPIO_PORT_LEVEL_MIN + (savedLevel * (GPIO_PORT_LEVEL_MAX - GPIO_PORT_LEVEL_MIN));
        setOutput(pinNumber, gpioLevel);
        return savedLevel;
    }
    float GPIOPort::off(uint8_t pinNumber) {
        setOutput(pinNumber, GPIO_PORT_LEVEL_MIN);
        return 0;
    }
    float GPIOPort::level(uint8_t pinNumber, float level) {
        float gpioLevel = GPIO_PORT_LEVEL_MIN + (level * (GPIO_PORT_LEVEL_MAX - GPIO_PORT_LEVEL_MIN) / 100);
        setOutput(pinNumber, gpioLevel);
        level = (gpioLevel - GPIO_PORT_LEVEL_MIN) / GPIO_PORT_LEVEL_MAX;
        return level;
    }
}}
