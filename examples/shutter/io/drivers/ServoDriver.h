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

#include <ESP32Servo.h>

#include <io/Logger.h>

#include "../../configuration.h"
#include "../../io/IShutterDriver.h"


#define SERVO_DRIVER_NS_PREFIX "IO::Components:ShutterControl::ServoDriver"

namespace IO { namespace Components {

    static bool ServoDriver_triggered;


        static int lastCommand = 0;
        static int revolutions = 0;
        static int revolutionsMax = 17;
        static bool stopRequested = false;
        static unsigned long lastTickMs = 0;
        static float currentLevel = 0;
        static float targetLevel = -1;
        static int direction = -1; // reverse open/close


        class ServoDriver: public Task, public IShutterDriver {
    private:
        String domain = IO::IOEventDomains::Automation_Components;
        String address = SERVO_MODULE_ADDRESS;
        Servo* servoDriver;

        const int servoPin = CONFIG_ServoMotorPin;

        const int frequency = 330; // Hz // Standard is 50(hz) servo
        int minUs = 500;
        int maxUs = 2500;
        int stopUs = 1500;
        int stepSpeed = 800; // [1 .. 1000] microseconds

        unsigned long lastEventMs = 0;

    public:
        ServoDriver() {}
        void init() override {
            Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", SERVO_DRIVER_NS_PREFIX, servoPin, minUs, maxUs);
            servoDriver = new Servo();
            servoDriver->setPeriodHertz(frequency);
            servoDriver->attach(servoPin, minUs, maxUs);

            ServoDriver_triggered = false;
            pinMode(3, INPUT_PULLUP);
            attachInterrupt(3, [] {

// TODO: should consider speed?
                ServoDriver_triggered = millis() - lastTickMs > 100;
                lastTickMs = millis();

                if (ServoDriver_triggered) {
                    if (lastCommand != SHUTTER_COMMAND_NONE) {
                        revolutions += (lastCommand == SHUTTER_COMMAND_OPEN ? 1 : -1);
                        float level =  (float)revolutions / (float)revolutionsMax;
                        if (revolutions <= 0 || revolutions >= revolutionsMax || (targetLevel != -1 && level == targetLevel)) {
                            stopRequested = true;
                        }
                    }
                }

            }, CHANGE);

            Logger::info("|  ✔ %s", SERVO_DRIVER_NS_PREFIX);
        }
        void calibrate() override {

            // TODO: ....

        }
        void speed(float s) override {
            // s ->  ]0.0 .. 1.0[
            stepSpeed = s * 1000;
        }
        void stop() override {
            servoDriver->write(stopUs);
        }
        void open() override {
            if (lastCommand != SHUTTER_COMMAND_NONE || revolutions == revolutionsMax) {
                stopRequested = true;
            } else {
                servoDriver->write(stopUs + stepSpeed * direction);
                lastCommand = SHUTTER_COMMAND_OPEN;
                lastTickMs = millis();
            }
        }
        void close() override {
            if (lastCommand != SHUTTER_COMMAND_NONE || revolutions == 0) {
                stopRequested = true;
            } else {
                servoDriver->write(stopUs - stepSpeed * direction);
                lastCommand = SHUTTER_COMMAND_CLOSE;
                lastTickMs = millis();
            }
        }
        void level(float level) override {
            targetLevel = round((level / 100.f) * revolutionsMax) / revolutionsMax;
            float levelDiff = targetLevel - currentLevel;
            if (levelDiff < 0) {
                close();
            } else if (levelDiff > 0) {
                open();
            }
        }

        void loop() override {

            if (lastCommand != SHUTTER_COMMAND_NONE && (millis() - lastTickMs > 1000)) {

                // MOTOR ERROR!!!!! DID NOT RECEIVE "PULSE" for 1000ms

                String err = "No pulse received from motor.";
                Logger::info("@%s [%s %s]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Error), err.c_str());
                eventSender->sendEvent((const uint8_t*)(IOEventPaths::Status_Error), &err, IOEventDataType::Text);

                stopRequested = true;

                // TODO: SHOULD GO BACK TO THE POSITION WHERE STARTED THE COMMAND
            }

            if (stopRequested) {

                stopRequested = false;
                stop();
                currentLevel = (float)revolutions / (float)revolutionsMax;
                lastCommand = SHUTTER_COMMAND_NONE;
                targetLevel = -1;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                eventSender->sendEvent((const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);

            } else if (lastCommand == SHUTTER_COMMAND_OPEN) {

                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level = (float)revolutions / (float)revolutionsMax;
                    lastEventMs = millis();
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                    eventSender->sendEvent((const uint8_t *) (IOEventPaths::Status_Level), &level, IOEventDataType::Float);
                }

            } else if (lastCommand == SHUTTER_COMMAND_CLOSE) {

                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level =  (float)revolutions / (float)revolutionsMax;
                    lastEventMs = millis();
                    Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                    eventSender->sendEvent((const uint8_t *) (IOEventPaths::Status_Level), &level, IOEventDataType::Float);
                }

            }

        }

    };
}}

#endif //HOMEGENIE_MINI_SERVODRIVER_H
