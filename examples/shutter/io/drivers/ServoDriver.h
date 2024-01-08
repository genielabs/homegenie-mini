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
    class ServoDriver: public IShutterDriver {
    private:
        Servo* servoDriver;
        const int frequency = 330; // Hz // Standard is 50(hz) servo
        const int servoPin = 15;
        int minUs = 500;
        int maxUs = 2500;
        int stopUs = 1500;
        int stepSpeed = 300; // [1 .. 1000] microseconds
    public:
        ServoDriver() {}
        void init() {
            Logger::info("|  - %s (PIN=%d MIN=%d MAX=%d)", SERVO_DRIVER_NS_PREFIX, servoPin, minUs, maxUs);
            servoDriver = new Servo();
            servoDriver->setPeriodHertz(frequency);
            servoDriver->attach(servoPin, minUs, maxUs);
            Logger::info("|  ✔ %s", SERVO_DRIVER_NS_PREFIX);
        }
        void stop() {
            servoDriver->write(stopUs);
        }
        void open() {
            servoDriver->write(stopUs + stepSpeed);
        }
        void close() {
            servoDriver->write(stopUs - stepSpeed);
        }
    };
}}

#endif //HOMEGENIE_MINI_SERVODRIVER_H
