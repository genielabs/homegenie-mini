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
 * - 2024-01-06 Initial release
 *
 */


// Note that with a 360 servo the accuracy of positioning isn't very precise,
// for a precise position seeking is required a stepper motor


#ifndef HOMEGENIE_MINI_SERVODRIVER_H
#define HOMEGENIE_MINI_SERVODRIVER_H

#include "examples/shutter/io/IShutterDriver.h"

#include ".pio/libdeps/shutter/ESP32Servo/src/ESP32Servo.h"

#include "io/Logger.h"

#define SERVO_DRIVER_NS_PREFIX "IO::Components:ShutterControl::ServoDriver"

namespace IO { namespace Components {
    class ServoDriver: public Task, public IShutterDriver {
    private:
        String domain = IO::IOEventDomains::Automation_Components;
        String address = SERVO_MODULE_ADDRESS;
        Servo* servoDriver;
        const int frequency = 330; // Hz // Standard is 50(hz) servo
        const int servoPin = 15;
        int minUs = 500;
        int maxUs = 2500;
        int stopUs = 1500;
        int stepSpeed = 300; // [1 .. 1000] microseconds

        int lastCommand = 0;
        long lastCommandTs = 0;
        unsigned long lastEventMs = 0;
        float currentLevel = 0;

        float totalTimeSpanMs = 20000.0f;
        bool stopRequested = false;
        float stopTime;

    public:
        ServoDriver() {}
        void init() override {
            Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", SERVO_DRIVER_NS_PREFIX, servoPin, minUs, maxUs);
            servoDriver = new Servo();
            servoDriver->setPeriodHertz(frequency);
            servoDriver->attach(servoPin, minUs, maxUs);
            Logger::info("|  ✔ %s", SERVO_DRIVER_NS_PREFIX);
        }
        void stop() override {
            servoDriver->write(stopUs);
        }
        void open() override {
            if (lastCommand != SHUTTER_COMMAND_NONE) {
                stopRequested = true;
            } else {
                servoDriver->write(stopUs + stepSpeed);
                lastCommand = SHUTTER_COMMAND_OPEN;
                lastCommandTs = millis();
            }
        }
        void close() override {
            if (lastCommand != SHUTTER_COMMAND_NONE) {
                stopRequested = true;
            } else {
                servoDriver->write(stopUs - stepSpeed);
                lastCommand = SHUTTER_COMMAND_CLOSE;
                lastCommandTs = millis();
            }
        }
        void level(float level) override {
            float levelDiff = (level / 100.f) - currentLevel;
            if (levelDiff < 0) {
                stopTime = millis() - (totalTimeSpanMs * levelDiff);
                close();
            } else if (levelDiff > 0) {
                stopTime = millis() + (totalTimeSpanMs * levelDiff);
                open();
            }
        }

        void loop() override {

            long elapsed = millis() - lastCommandTs;
            float percent = ((float)elapsed / totalTimeSpanMs);
            if (stopRequested || (stopTime != 0 && millis() >= stopTime)) {

                stopRequested = false;
                stopTime = 0;
                stop();
                if (lastCommand == SHUTTER_COMMAND_OPEN) {
                    currentLevel += percent;
                } else if (lastCommand == SHUTTER_COMMAND_CLOSE) {
                    currentLevel -= percent;
                }
                lastCommand = SHUTTER_COMMAND_NONE;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);

            } else if (lastCommand == SHUTTER_COMMAND_OPEN) {

                if (elapsed > (totalTimeSpanMs - (currentLevel * totalTimeSpanMs))) {
                    lastCommand = SHUTTER_COMMAND_NONE;
                    stop();
                    currentLevel = 1;
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                    eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                              &currentLevel, IOEventDataType::Float);
                } else if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level = currentLevel + percent;
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                    eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                              &level, IOEventDataType::Float);
                }

            } else if (lastCommand == SHUTTER_COMMAND_CLOSE) {

                if (elapsed > (currentLevel * totalTimeSpanMs)) {
                    lastCommand = SHUTTER_COMMAND_NONE;
                    stop();
                    currentLevel = 0;
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                    eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);
                } else if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level = currentLevel - percent;
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                    eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                              &level, IOEventDataType::Float);
                }

            }

        }

    };
}}

#endif //HOMEGENIE_MINI_SERVODRIVER_H
