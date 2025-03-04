/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

#include "MotionSensor.h"

namespace IO { namespace Sensors {
    void MotionSensor::begin() {
        pinMode(sensorPin, INPUT_PULLUP);
        Logger::info("|  ✔ %s (PIN=%d)", MOTION_SENSOR_NS_PREFIX, sensorPin);
    }
    void MotionSensor::loop() {
        int motionValue = digitalRead(sensorPin);
        if (motionValue == HIGH && !motionDetected) {
            clearIdle();
            // MOTION DETECTED
            motionDetected = true;
            Logger::info("@%s [%s %d]", MOTION_SENSOR_NS_PREFIX, IOEventPaths::Sensor_MotionDetect, motionValue);
            sendEvent(IOEventPaths::Sensor_MotionDetect, &motionValue, IOEventDataType::Number);
#ifdef CONFIG_ENABLE_POWER_MANAGER
            PowerManager::setActive();
#endif
        } else if (motionValue == LOW && motionDetected) {
            clearIdle();
            // MOTION CLEAR
            motionDetected = false;
            Logger::info("@%s [%s %d]", MOTION_SENSOR_NS_PREFIX, IOEventPaths::Sensor_MotionDetect, motionValue);
            sendEvent(IOEventPaths::Sensor_MotionDetect, &motionValue, IOEventDataType::Number);
        } else if (motionDetected) {
            // MOTION ACTIVE
#ifdef CONFIG_ENABLE_POWER_MANAGER
            PowerManager::setActive();
#endif
        }

        updateIdle();
    }
    void MotionSensor::clearIdle() {
        idleTime = millis();
        lastUpdate = idleTime - 60000; // forces update
        updateIdle();
    }
    void MotionSensor::updateIdle() {
        uint32_t elapsed = millis() - lastUpdate;
        if (elapsed >= 60000) {
            lastUpdate = millis();
            uint32_t idleMinutes = (millis() - idleTime) / 60000;
            sendEvent(IOEventPaths::Status_IdleTime, &idleMinutes, IOEventDataType::Number);
        }
    }
}} // Sensors
