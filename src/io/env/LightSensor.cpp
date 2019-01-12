//
// Created by gene on 11/01/19.
//

#include "LightSensor.h"

namespace IO { namespace Env {

        void LightSensor::begin() {
            Logger::info("|  ✔ IO::Env::LightSensor");
        }

        void LightSensor::loop() {
            static unsigned long lastSampleTime = 0 - samplingRate;
            unsigned long now = millis();
            if (now - lastSampleTime >= samplingRate) {
                float lightLevel = getLightLevel();
                Logger::info("@IO::Env::LightSensor %0.2f", lightLevel);
                lastSampleTime = now;
            }
        }

        void LightSensor::setInputPin(uint8_t number) {
            inputPin = number;
        }

        float LightSensor::getLightLevel() {
            // It returns values between 0-1024
            return analogRead(inputPin);
        }
    }}
