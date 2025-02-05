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

#ifndef HOMEGENIE_MINI_SHUTTERAPI_H
#define HOMEGENIE_MINI_SHUTTERAPI_H

#include <Config.h>

#define K Config::getKey

namespace Service { namespace API {

    namespace ShutterApi {

        const char Shutter_Calibrate[] =
                "Shutter.Calibrate";

        namespace Options {
            const char Shutter_Motor[] =
                    "Shutter.Motor";
            const char Shutter_Motor_Servo_AngleMin[] =
                    "Shutter.Motor.Servo.AngleMin";
            const char Shutter_Motor_Servo_AngleMax[] =
                    "Shutter.Motor.Servo.AngleMax";
            const char Shutter_Motor_Servo_AngleSpeed[] =
                    "Shutter.Motor.Servo.AngleSpeed";
            const char Shutter_Motor_ServoEncoder_Speed[] =
                    "Shutter.Motor.ServoEncoder.Speed";
            const char Shutter_Motor_ServoEncoder_Steps[] =
                    "Shutter.Motor.ServoEncoder.Steps";
            const char Shutter_Motor_ServoEncoder_Calibrate[] =
                    "Shutter.Motor.ServoEncoder.Calibrate";
        }

        namespace Configuration {
            static const char MotorType[] = "motr-typ";
            static const char ControlPin[] = "ctrl-pin";
            namespace MotorTypeValues {
                static const char Servo[] = "Servo";
                static const char ServoEncoder[] = "ServoEncoder";
                static const char Stepper[] = "Stepper";
            }
            namespace Servo {
                static const char AngleSpeed[] = "angl-spd";
                static const char AngleMin[] = "angl-min";
                static const char AngleMax[] = "angl-max";
            }
            namespace ServoEncoder {
                static const char EncoderPin[] = "encd-pin";
                static const char MotorSpeed[] = "motr-spd";
                static const char MotorSteps[] = "motr-stp";
            }
        }

        namespace Calibration {
            const char StartLabel[] =
                    "start_calibration";
            const char EndLabel[] =
                    "end_calibration";
        }

    }

}}

#endif //HOMEGENIE_MINI_SHUTTERAPI_H
