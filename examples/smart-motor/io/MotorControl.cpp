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

#include "MotorControl.h"

namespace IO { namespace Components {

    MotorControl::MotorControl(int i) {
        idx = i;
    }

    void MotorControl::begin() {
        // read current config
        auto type = K(MotorType, idx);
        auto cfgMotorType = Config::getSetting(type, "Servo");
        configure(type, cfgMotorType.c_str());
    }

    void MotorControl::open() {
        motorDriver->open();
    }
    void MotorControl::close() {
        motorDriver->close();
    }
    void MotorControl::setLevel(float level) {
        motorDriver->level(level);
    }
    void MotorControl::toggle() {
        motorDriver->toggle();
    }

    void MotorControl::configure(const char* k, const char* v) {
        auto key = String(k);
        if (key.equals(K(MotorType, idx))) {
            setType(v);
        } else {
            motorDriver->configure(k, v);
        }
    }

    void MotorControl::setType(const char* t) {
        String type = t;
        if (motorDriver != nullptr) {
            motorDriver->release();
            motorDriver = nullptr;
        }
        auto controlPin = Config::getSetting(K(ControlPin, idx), "-1").toInt();
        if (controlPin < 0) return;
        if (type == MotorTypeValues::Stepper) {
// TODO: read pins config
            motorDriver = new StepperDriver(/* controlPin, ..., index */);
        } else if (type == MotorTypeValues::ServoEncoder) {
            auto encoderPin = Config::getSetting(K(EncoderPin, idx), "-1").toInt();
            motorDriver = new ServoEncoderDriver(controlPin, encoderPin, idx);
        } else {
            motorDriver = new ServoDriver(controlPin, idx);
        }
        motorDriver->eventSender = this;
        motorDriver->init();
    }

    void MotorControl::calibrate() {
        calibrateMode = !calibrateMode;
        motorDriver->calibrate(calibrateMode);
    }

}}
