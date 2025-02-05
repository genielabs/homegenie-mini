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
 * - 2024-01-08 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_STEPPERDRIVER_H
#define HOMEGENIE_MINI_STEPPERDRIVER_H

#include <HomeGenie.h>
#include <Stepper.h>

#include "../IShutterDriver.h"

#define STEPPER_DRIVER_NS_PREFIX "IO::Components:ShutterControl::StepperDriver"

// TODO: this is a work in progress/draft... hard coded stuff to be re-arranged

// ULN2003 Motor Driver Pins
#define IN1 19
#define IN2 18
#define IN3 5
#define IN4 17

namespace IO { namespace Components {
    enum StepperDirection {
        STEPPER_DIRECTION_NONE,
        STEPPER_DIRECTION_CLOCKWISE,
        STEPPER_DIRECTION_COUNTERCLOCKWISE
    };
    class StepperDriver: public Task, public IShutterDriver {
    private:
        const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
        int currentPosition = 0;
        int stepIncrement = 1;
        int maxPosition = stepsPerRevolution * 5;
        int seekToPosition = -1; // -1 = disabled
        unsigned long lastEventMs = 0;
        StepperDirection currentMotion = STEPPER_DIRECTION_NONE;
        Stepper* myStepper;

        bool isCalibrating = false;

        void sendLevel() {
            float currentLevel = ((float)currentPosition / (float)maxPosition);
            Logger::info("@%s [%s %.4f]", STEPPER_DRIVER_NS_PREFIX, IOEventPaths::Status_Level, currentLevel);
            eventSender->sendEvent(IOEventPaths::Status_Level, &currentLevel, IOEventDataType::Float);
        }

    public:
        StepperDriver() {
        }
        ~StepperDriver() override {
            delete myStepper;
        }
        void init() override {
            //Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", STEPPER_DRIVER_NS_PREFIX, servoPin, minUs, maxUs);
            myStepper = new Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
            myStepper->setSpeed(12);
            Logger::info("|  ✔ %s", STEPPER_DRIVER_NS_PREFIX);
        }
        void loop() override {
            if (currentMotion != STEPPER_DIRECTION_NONE) {
                int inc = currentMotion == STEPPER_DIRECTION_CLOCKWISE ? stepIncrement : -stepIncrement;
                currentPosition += inc;
                if (currentPosition < 0 || currentPosition > maxPosition) {
                    currentPosition = currentPosition >= maxPosition ? maxPosition : 0;
                    stop();
                    return;
                } else {
                    myStepper->step(inc);
                }

                if (currentPosition % stepsPerRevolution == 0) {
                    Serial.print("\n.\n");
                }

                if (seekToPosition != -1 && (
                        (currentMotion == STEPPER_DIRECTION_CLOCKWISE && currentPosition >= seekToPosition)
                            || (currentMotion == STEPPER_DIRECTION_COUNTERCLOCKWISE && currentPosition <= seekToPosition))) {
                    stop();
                }
                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    lastEventMs = millis();
                    sendLevel();
                }
            }
        }
        void calibrate(bool enable) override {
            isCalibrating = enable;
        }
        void configure(const char* k, const char* v) override {
            // TODO: ..............
        }

        void open() override {
            if (currentMotion != STEPPER_DIRECTION_NONE) stop();
            else currentMotion = STEPPER_DIRECTION_CLOCKWISE;
        }
        void close() override {
            if (currentMotion != STEPPER_DIRECTION_NONE) stop();
            else currentMotion = STEPPER_DIRECTION_COUNTERCLOCKWISE;
        }
        void level(float level) override {
            seekToPosition = round((float)maxPosition / 100.0f * level);
            if (currentPosition < seekToPosition) {
                open();
            } else if (currentPosition > seekToPosition) {
                close();
            }
        }
        void toggle() override {
            // TODO: ..............
        }

        void stop() override {
            currentMotion = STEPPER_DIRECTION_NONE;
            seekToPosition = -1;
            sendLevel();
        }
    };
}}

#endif //HOMEGENIE_MINI_STEPPERDRIVER_H
