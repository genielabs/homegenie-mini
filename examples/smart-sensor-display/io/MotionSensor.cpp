//
// Created by gene on 19/02/24.
//

#include "MotionSensor.h"

namespace IO { namespace Env {
    void MotionSensor::begin() {
        pinMode(sensorPin, INPUT_PULLUP);
        Logger::info("|  ✔ %s (PIN=%d)", MOTION_SENSOR_NS_PREFIX, sensorPin);
    }
    void MotionSensor::loop() {
        int motionValue = digitalRead(sensorPin);
        if (motionValue == HIGH && !motionDetected) {
            // MOTION DETECTED
            motionDetected = true;
            Logger::info("@%s [%s %d]", MOTION_SENSOR_NS_PREFIX, (IOEventPaths::Sensor_MotionDetect), motionValue);
            sendEvent((const uint8_t*)(IOEventPaths::Sensor_MotionDetect), (int*)&motionValue, IOEventDataType::Number);
#ifdef CONFIG_ENABLE_POWER_MANAGER
            PowerManager::setActive();
#endif
        } else if (motionValue == LOW && motionDetected) {
            // MOTION CLEAR
            motionDetected = false;
            Logger::info("@%s [%s %d]", MOTION_SENSOR_NS_PREFIX, (IOEventPaths::Sensor_MotionDetect), motionValue);
            sendEvent((const uint8_t*)(IOEventPaths::Sensor_MotionDetect), (int*)&motionValue, IOEventDataType::Number);
        } else if (motionDetected) {
            // MOTION ACTIVE
#ifdef CONFIG_ENABLE_POWER_MANAGER
            PowerManager::setActive();
#endif
        }
    }
}} // Env
