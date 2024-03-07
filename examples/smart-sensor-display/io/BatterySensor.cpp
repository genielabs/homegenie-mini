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
 */

#include "BatterySensor.h"

namespace IO { namespace Env {

    void  BatterySensor::begin() {
        pinMode(sensorPin, INPUT);
    }
    void BatterySensor::loop() {
        float gpioMax = 4096;
        float gpioV = 3.3f;
        float batteryVMax = 3.0f;
        const float conversionFactor = gpioV / gpioMax * batteryVMax;
        float adv = analogReadMilliVolts(sensorPin);
        float v = adv * conversionFactor; //adv * (gpioV / gpioMax) * batteryV;
        float batteryLevel = v / batteryVMax * 100;
        if (lastBatteryLevel != batteryLevel) {
            Logger::info("@%s [%s %.1f] (%.1f)", BATTERY_SENSOR_NS_PREFIX, (IOEventPaths::Status_Battery), batteryLevel, adv);
            sendEvent((const uint8_t*)(IOEventPaths::Status_Battery), (float*)&batteryLevel, IOEventDataType::Float);
            lastBatteryLevel = batteryLevel;
        }

        if (batteryLevel > 100) {
            // Charging
            Service::PowerManager::setActive();
        }
    }

}}
