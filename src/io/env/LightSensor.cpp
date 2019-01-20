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

#include "LightSensor.h"

namespace IO { namespace Env {

    void LightSensor::begin() {
        Logger::info("|  ✔ %s", LIGHTSENSOR_NS_PREFIX);
    }

    void LightSensor::loop() {
        uint16_t lightLevel = getLightLevel();
        // signal value changes
        if (lightLevel != currentLevel) {
            currentLevel = lightLevel;
            Logger::info("@%s [%s %d]", LIGHTSENSOR_NS_PREFIX, IOEventPaths::Sensor_Luminance, currentLevel);
            sendEvent((uint8_t*)IOEventPaths::Sensor_Luminance, (uint16_t *)&currentLevel, SensorLight);
        }
    }

    void LightSensor::setInputPin(const uint8_t number) {
        inputPin = number;
    }

    uint16_t LightSensor::getLightLevel() {
        // It returns values between 0-1024
        return (uint16_t)analogRead(inputPin);
    }
}}
