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

#ifndef HOMEGENIE_MINI_SERVOENCODERDRIVER_H
#define HOMEGENIE_MINI_SERVOENCODERDRIVER_H

#include <ESP32Servo.h>

#include <io/Logger.h>

#include "../../configuration.h"
#include "../../api/MotorApi.h"
#include "../IMotorDriver.h"


#define SERVO_ENCODER_DRIVER_NS_PREFIX "IO::Components:MotorControl::ServoEncoderDriver"

namespace IO { namespace Components {

    using namespace Service::API::MotorApi;
    using namespace Configuration::ServoEncoder;

    class ServoEncoderDriver: public Task, public IMotorDriver {
    private:
        Servo* servoDriver;

        int servoPin = -1;
        int gpioEncoderPin = -1;
        int idx;

        const int frequency = 50; // Hz // Standard is 50(hz) servo
        int minUs = 500;
        int maxUs = 2500;
        int stopUs = 1500;
        int stepSpeed = 500; // [1 .. 1000] microseconds

        unsigned long lastEventMs = 0;
        unsigned int minPulseDuration = 30;
        unsigned int maxPulseTimeout = 3000;

        bool ServoEncoderDriver_triggered;

        int lastCommand = 0;
        int stepsCount = 0;
        int stepsMax = 15;
        bool stopRequested = false;
        unsigned long lastTickMs = 0;
        float currentLevel = 0;
        float targetLevel = -1;
        int direction = -1; // reverse open/close

        bool isCalibrating = false;

        bool hasGpioEncoder() const {
            return gpioEncoderPin >= 0;
        }
        void reportEncoderError() {
            String err = "No pulse received from motor.";
            Logger::info("@%s [%s %s]", SERVO_ENCODER_DRIVER_NS_PREFIX, (IOEventPaths::Status_Error),
                         err.c_str());
            eventSender->sendEvent(IOEventPaths::Status_Error, &err, IOEventDataType::Text);
            stopRequested = true;
        }

        unsigned int getTimeoutMs() const {
            return maxPulseTimeout - (stepSpeed * 1.5);
        }

        void gpioEncoderHandler(int speed = 0) {

            if (!hasGpioEncoder()) {
                // Simulate step pulse frequency
                ServoEncoderDriver_triggered = millis() - lastTickMs >= (1500.0 - speed) / 4.0f;
            }

            if (lastCommand != MOTOR_COMMAND_NONE) {

                if (hasGpioEncoder()) {
                    // encoder switch is pressed
                    if (digitalRead(gpioEncoderPin) == LOW) {
                        lastTickMs = millis();
                        // wait until it's released for max. `maxPulseTimeout` ms
                        while (digitalRead(gpioEncoderPin) == LOW) {
                            if (millis() - lastTickMs > getTimeoutMs()) break;
                        }
                        auto pulseDuration = (millis() - lastTickMs);
                        if (pulseDuration > getTimeoutMs()) {
                            // The encoder switch was held down for too long,
                            // a mechanical issue might have occurred.
                            ServoEncoderDriver_triggered = false;
                            reportEncoderError();
                        } else if (pulseDuration > minPulseDuration) {
                            // Encoder pulse received, trigger step count inc./dec.
                            ServoEncoderDriver_triggered = true;
                        } else {
                            // The pulse was too short, might have been just fluctuations (?)
                            ServoEncoderDriver_triggered = false;
                        }
                    } else {
                        // No pulse received from encoder, a mechanical
                        // or electronics issue might have occurred.
                        ServoEncoderDriver_triggered = false;
                        auto elapsed = millis() - lastTickMs;
                        if (elapsed > getTimeoutMs()) {
                            reportEncoderError();
                        }
                    }

                }

                if (ServoEncoderDriver_triggered) {
                    stepsCount += (lastCommand == MOTOR_COMMAND_OPEN ? 1 : -1);
                    if (isCalibrating) {
                        // Allow only one step per-command
                        // if calibration mode is active to
                        // allow adjusting upper/lower limits
                        // of the motor motion range.
                        if (stepsCount < 0) {
                            stepsCount = 0;
                        } else if (stepsCount > stepsMax) {
                            stepsMax = stepsCount;
                        }
                    }
                    // Check if target position has been reached
                    // and stop the motor.
                    float level = (float) stepsCount / (float) stepsMax;
                    if (stepsCount <= 0
                        || stepsCount >= stepsMax
                        || (targetLevel != -1 && direction * (level - targetLevel) >= 0)) {

                        stopRequested = true;

                    }
                    lastTickMs = millis();
                }

            }

        }

    public:
        explicit ServoEncoderDriver(int signalPin, int encoderPin, int index) {
            servoPin = signalPin;
            gpioEncoderPin = encoderPin;
            this->idx = index;
            setLoopInterval(10);
        }
        void release() override {
            setDisposed();
            servoDriver->release();
            servoDriver->detach();
            delete servoDriver;
        }

        void init() override {
            Logger::info("|  - %s (PIN=%d ENC=%d MIN=%d MAX=%d)", SERVO_ENCODER_DRIVER_NS_PREFIX, servoPin, gpioEncoderPin, minUs, maxUs);
            servoDriver = new Servo();
            servoDriver->setPeriodHertz(frequency);
            servoDriver->attach(servoPin, minUs, maxUs);

            // Setup GPIO encoder
            ServoEncoderDriver_triggered = false;
            if (hasGpioEncoder()) {
                pinMode(gpioEncoderPin, INPUT_PULLUP);
            }

            // Read stored config or apply default values
            auto k = K(MotorSpeed, idx);
            configure(k, Config::getSetting(k, "5").c_str());
            k = K(MotorSteps, idx);
            configure(k, Config::getSetting(k, "15").c_str());

            Logger::info("|  ✔ %s", SERVO_ENCODER_DRIVER_NS_PREFIX);
        }

        void configure(const char* k, const char* v) override {
            auto key = String(k);
            auto value = String(v);
            if (key.equals(K(MotorSpeed, idx))) {
                stepSpeed = (int)round(value.toFloat() * 100);
            } else if (key.equals(K(MotorSteps, idx))) {
                stepsMax = value.toInt();
                if (stepsCount > stepsMax) {
                    stepsCount = stepsMax;
                }
                stopRequested = true;
            }
            Logger::info("@%s::configure [%s = %s]", SERVO_ENCODER_DRIVER_NS_PREFIX, key.c_str(), value.c_str());
        }

        void calibrate(bool enable) override {
            isCalibrating = enable;
            if (isCalibrating) {
                stepsCount = 0;
                stepsMax = 1;
                eventSender->sendEvent(Options::Motor_ServoEncoder_Steps,
                                       &stepsMax,
                                       IOEventDataType::Number);
                String endCalibrationText = Calibration::EndLabel;
                eventSender->sendEvent(Options::Motor_ServoEncoder_Calibrate,
                                       &endCalibrationText,
                                       IOEventDataType::Text);
                int level = 0;
                eventSender->sendEvent(IOEventPaths::Status_Level,
                                       &level,
                                       IOEventDataType::Number);
            } else {
                String endCalibrationText = Calibration::StartLabel;
                eventSender->sendEvent(Options::Motor_ServoEncoder_Calibrate,
                                       &endCalibrationText,
                                       IOEventDataType::Text);
            }
        }

        void open() override {
            if (lastCommand != MOTOR_COMMAND_NONE || (stepsCount == stepsMax && !isCalibrating)) {
                stopRequested = true;
            } else {
                direction = 1;
                lastTickMs = millis();
                lastCommand = MOTOR_COMMAND_OPEN;
                // The next lines work when calibration is active
                // to adjust end level (fully open)
                if (stepsCount == stepsMax) {
                    stepsMax++;
                    eventSender->sendEvent(Options::Motor_ServoEncoder_Steps,
                                           &stepsMax,
                                           IOEventDataType::Number);
                }
            }
        }
        void close() override {
            if (lastCommand != MOTOR_COMMAND_NONE || (stepsCount == 0 && !isCalibrating)) {
                stopRequested = true;
            } else {
                direction = -1;
                lastTickMs = millis();
                lastCommand = MOTOR_COMMAND_CLOSE;
                // The next line works when calibration is active
                // to adjust start position (fully shut)
                if (stepsCount == 0) stepsCount++;
            }
        }
        void level(float level) override {
            targetLevel = (((level / 100.f) * (float)stepsMax) / (float)stepsMax);
            float levelDiff = targetLevel - currentLevel;
            if (levelDiff < 0) {
                direction = -1;
                lastTickMs = millis();
                lastCommand = MOTOR_COMMAND_CLOSE;
                // The next line works when calibration is active
                // to adjust start position (fully shut)
                if (stepsCount == 0) stepsCount++;
            } else if (levelDiff > 0) {
                direction = 1;
                lastTickMs = millis();
                lastCommand = MOTOR_COMMAND_OPEN;
                // The next lines work when calibration is active
                // to adjust end level (fully open)
                if (stepsCount == stepsMax) {
                    stepsMax++;
                    eventSender->sendEvent(Options::Motor_ServoEncoder_Steps,
                                           &stepsMax,
                                           IOEventDataType::Number);
                }
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
            servoDriver->writeMicroseconds(stopUs);
        }

        void loop() override {

            // simulate encoder interrupt
            gpioEncoderHandler(stepSpeed);

            if (stopRequested) {

                stopRequested = false;
                stop();
                currentLevel = (float) stepsCount / (float) stepsMax;
                lastCommand = MOTOR_COMMAND_NONE;
                targetLevel = -1;
                Logger::info("@%s [%s %.2f]", SERVO_ENCODER_DRIVER_NS_PREFIX, (IOEventPaths::Status_Level),
                             currentLevel);
                eventSender->sendEvent(IOEventPaths::Status_Level, &currentLevel, IOEventDataType::Float);

            } else if (lastCommand == MOTOR_COMMAND_OPEN) {

                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level = (float) stepsCount / (float) stepsMax;
                    if (level != currentLevel) {
                        currentLevel = level;
                        lastEventMs = millis();
                        Logger::info("@%s [%s %.2f]", SERVO_ENCODER_DRIVER_NS_PREFIX, (IOEventPaths::Status_Level),
                                     level);
                        eventSender->sendEvent(IOEventPaths::Status_Level, &level, IOEventDataType::Float);
                    }
                }

            } else if (lastCommand == MOTOR_COMMAND_CLOSE) {

                if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                    float level = (float) stepsCount / (float) stepsMax;
                    if (level != currentLevel) {
                        currentLevel = level;
                        lastEventMs = millis();
                        Logger::info("@%s [%s %.2f]", SERVO_ENCODER_DRIVER_NS_PREFIX, (IOEventPaths::Status_Level),
                                     level);
                        eventSender->sendEvent(IOEventPaths::Status_Level, &level, IOEventDataType::Float);
                    }
                }

            }

            if (lastCommand != MOTOR_COMMAND_NONE) {
                servoDriver->writeMicroseconds(stopUs + (stepSpeed * direction));
            }

        }

    };
}}

#endif //HOMEGENIE_MINI_SERVOENCODERDRIVER_H
