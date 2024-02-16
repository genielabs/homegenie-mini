//
// Created by gene on 08/01/24.
//

#ifndef HOMEGENIE_MINI_STEPPERDRIVER_H
#define HOMEGENIE_MINI_STEPPERDRIVER_H

#include <HomeGenie.h>
#include <Stepper.h>

#include "../IShutterDriver.h"

#define STEPPER_DRIVER_NS_PREFIX "IO::Components:ShutterControl::StepperDriver"

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
        String domain = IO::IOEventDomains::Automation_Components;
        String address = SERVO_MODULE_ADDRESS;
        const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
        int currentPosition = 0;
        int stepIncrement = 1;
        int maxPosition = stepsPerRevolution * 5;
        int seekToPosition = -1; // -1 = disabled
        unsigned long lastEventMs = 0;
        StepperDirection currentMotion = STEPPER_DIRECTION_NONE;
        Stepper* myStepper;
    public:
        StepperDriver() {}
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
        void stop() override {
            currentMotion = STEPPER_DIRECTION_NONE;
            seekToPosition = -1;
            sendLevel();
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
        void sendLevel() {
            float currentLevel = ((float)currentPosition / (float)maxPosition);
            Logger::info("@%s [%s %.4f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
            eventSender->sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);
        }
    };
}}

#endif //HOMEGENIE_MINI_STEPPERDRIVER_H
