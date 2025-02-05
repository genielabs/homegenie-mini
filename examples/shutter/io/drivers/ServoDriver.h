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
 * - 2024-01-06 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_SERVODRIVER_H
#define HOMEGENIE_MINI_SERVODRIVER_H

#include <ESP32Servo.h>

#include <io/Logger.h>

#include "../../configuration.h"
#include "../../io/IShutterDriver.h"


#define SERVO_DRIVER_NS_PREFIX "IO::Components:ShutterControl::ServoDriver"

namespace IO { namespace Components {

    using namespace Service::API::ShutterApi;
    using namespace Configuration::Servo;

    class ServoDriver: public Task, public IShutterDriver {
    private:
        Servo* servoDriver;

        int servoPin;
        int idx;

        const int frequency = 50; // Hz // Standard is 50(hz) servo

        int minUs = 500;
        int maxUs = 2500;
        int stopUs = 1500;

        const int stepSpeedMax = 10;
        int stepSpeed = 5; // [1 .. <stepSpeedMax>]

        float minAngle = 0;
        float maxAngle = 180;

        int lastCommand = 0;
        float currentLevel = 0;
        float targetLevel = -1;
        int direction = -1; // reverse open/close

        unsigned long lastEventMs = 0;

        bool isCalibrating = false;
        bool stopRequested = false;

    public:
        ServoDriver(int pin, int index) {
            servoPin = pin;
            this->idx = index;
        }
        ~ServoDriver() override {
            servoDriver->release();
            delete servoDriver;
        }

        void init() override {
            Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", SERVO_DRIVER_NS_PREFIX, servoPin, minUs, maxUs);
            servoDriver = new Servo();
            servoDriver->setPeriodHertz(frequency);
            servoDriver->attach(servoPin, minUs, maxUs);

            // Read stored config or apply default values.
            auto k = K(AngleSpeed, idx);
            configure(k, Config::getSetting(k, "5").c_str());
            k = K(AngleMin, idx);
            configure(k, Config::getSetting(k, "0").c_str());
            k = K(AngleMax, idx);
            configure(k, Config::getSetting(k, "180").c_str());

            Logger::info("|  ✔ %s", SERVO_DRIVER_NS_PREFIX);
        }
        void calibrate(bool enable) override {
            // Calibration has no effect on this driver
            isCalibrating = enable;
        }
        void configure(const char* k, const char* v) override {
            auto key = String(k);
            auto value = String(v);
            if (key.equals(K(AngleSpeed, idx))) {
                stepSpeed = value.toInt();
                if (stepSpeedMax - stepSpeed > 0) {
                    setLoopInterval(1 + (float)(stepSpeedMax - stepSpeed - 1) * 3);
                } else {
                    setLoopInterval(10);
                }
            } else if (key.equals(K(AngleMin, idx))) {
                minAngle = value.toFloat();
            } else if (key.equals(K(AngleMax, idx))) {
                maxAngle = value.toFloat();
            }
            Logger::info("@%s::configure [%s = %s]", SERVO_DRIVER_NS_PREFIX, key.c_str(), value.c_str());
        }

        void open() override {
            if (lastCommand != SHUTTER_COMMAND_NONE) {
                stopRequested = true;
            } else {
                direction = 1;
                targetLevel = 1;
                lastCommand = SHUTTER_COMMAND_OPEN;
            }
        }
        void close() override {
            if (lastCommand != SHUTTER_COMMAND_NONE) {
                stopRequested = true;
            } else {
                direction = -1;
                targetLevel = 0;
                lastCommand = SHUTTER_COMMAND_CLOSE;
            }
        }
        void level(float level) override {
            targetLevel = level / 100.f;
            float levelDiff = targetLevel - currentLevel;
            if (levelDiff < 0) {
                direction = -1;
                lastCommand = SHUTTER_COMMAND_CLOSE;
            } else if (levelDiff > 0) {
                direction = 1;
                lastCommand = SHUTTER_COMMAND_OPEN;
            }
        }
        void toggle() override {
            if (currentLevel > 0) {
                close();
            } else {
                open();
            }
        }

        void stop() override {
            servoDriver->write(stopUs);
        }

        void loop() override {

            if (stopRequested) {
                stopRequested = false;
                lastCommand = SHUTTER_COMMAND_NONE;
                targetLevel = currentLevel;
                Logger::info("@%s [%s %.2f]", SERVO_DRIVER_NS_PREFIX, (IOEventPaths::Status_Level),
                             currentLevel);
                eventSender->sendEvent(IOEventPaths::Status_Level, &currentLevel, IOEventDataType::Float);
            }

            if (currentLevel != targetLevel && targetLevel >= 0) {

                currentLevel += (.00555f * direction);
                if ((stepSpeedMax - stepSpeed == 0)
                    || (direction > 0 && currentLevel > targetLevel)
                    || (direction < 0 && currentLevel < targetLevel)) {
                    currentLevel = targetLevel;
                    lastEventMs = 0;
                }

                if (currentLevel > 1) currentLevel = 1;
                if (currentLevel < 0) currentLevel = 0;

                // treated as angle if value < MIN_PULSE_WIDTH
                servoDriver->write((int)(round((maxAngle - minAngle) * currentLevel) + minAngle));


                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    lastEventMs = millis();
                    Logger::info("@%s [%s %.2f]", SERVO_DRIVER_NS_PREFIX, (IOEventPaths::Status_Level),
                                 currentLevel);
                    eventSender->sendEvent(IOEventPaths::Status_Level, &currentLevel, IOEventDataType::Float);
                }


                if (currentLevel == 1 || currentLevel == 0 || currentLevel == targetLevel) {
                    currentLevel = targetLevel;
                    stopRequested = true;
                    lastCommand = SHUTTER_COMMAND_NONE;
                }
            }

        }

    };
}}

#endif //HOMEGENIE_MINI_SERVODRIVER_H
