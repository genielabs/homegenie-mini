//
// Created by gene on 19/02/24.
//

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
