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
 * - 2019-01-19 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_IOEVENTPATHS_H
#define HOMEGENIE_MINI_IOEVENTPATHS_H

namespace IO {
    namespace IOEventPaths {
        const char Program_Status[]  = "Program.Status";
        const char Receiver_RawData[]  = "Receiver.RawData";
        const char Receiver_Command[]  = "Receiver.Command";
        const char Sensor_Luminance[]  = "Sensor.Luminance";
        const char Sensor_ColorHsv[]  = "Sensor.ColorHsv";
        const char Sensor_Temperature[]  = "Sensor.Temperature";
        const char Sensor_Humidity[]  = "Sensor.Humidity";
        const char Sensor_MotionDetect[]  = "Sensor.MotionDetect";
        const char Sensor_Orientation[]  = "Sensor.Orientation";
        const char Status_Level[]  = "Status.Level";
        const char Status_ColorHsb[]  = "Status.ColorHsb";
        const char Status_IdleTime[]  = "Status.IdleTime";
        const char Status_Battery[]  = "Status.Battery";
        const char Status_Error[]  = "Status.Error";
        const char System_Status[]  = "System.Status";
        const char System_BytesFree[]  = "System.BytesFree";
        const char Events_Disabled[]  = "Events.Disable";
    }
}


#endif //HOMEGENIE_MINI_IOEVENTPATHS_H
