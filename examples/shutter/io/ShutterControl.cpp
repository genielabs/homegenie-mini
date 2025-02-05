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

#include "ShutterControl.h"

namespace IO { namespace Components {

    ShutterControl::ShutterControl(int i) {
        idx = i;
    }

    void ShutterControl::begin() {
        // read current config
        auto type = K(MotorType, idx);
        auto cfgMotorType = Config::getSetting(type, "Servo");
        configure(type, cfgMotorType.c_str());
    }

    void ShutterControl::open() {
        shutterDriver->open();
    }
    void ShutterControl::close() {
        shutterDriver->close();
    }
    void ShutterControl::setLevel(float level) {
        shutterDriver->level(level);
    }
    void ShutterControl::toggle() {
        shutterDriver->toggle();
    }

    void ShutterControl::configure(const char* k, const char* v) {
        auto key = String(k);
        if (key.equals(K(MotorType, idx))) {
            setType(v);
        } else {
            shutterDriver->configure(k, v);
        }
    }

    void ShutterControl::setType(const char* t) {
        String type = t;
        delete shutterDriver;
        shutterDriver = nullptr;
        auto controlPin = Config::getSetting(K(ControlPin, idx), "-1").toInt();
        if (controlPin < 0) return;
        if (type == MotorTypeValues::Stepper) {
// TODO: read pins config
            shutterDriver = new StepperDriver(/* controlPin, ..., index */);
        } else if (type == MotorTypeValues::ServoEncoder) {
            auto encoderPin = Config::getSetting(K(EncoderPin, idx), "-1").toInt();
            shutterDriver = new ServoEncoderDriver(controlPin, encoderPin, idx);
        } else {
            shutterDriver = new ServoDriver(controlPin, idx);
        }
        shutterDriver->eventSender = this;
        shutterDriver->init();
    }

    void ShutterControl::calibrate() {
        calibrateMode = !calibrateMode;
        shutterDriver->calibrate(calibrateMode);
    }

}}
