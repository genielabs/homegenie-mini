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
        const char Events_Disabled[]     PROGMEM = "Events.Disable";
        const char Program_Status[]      PROGMEM = "Program.Status";
        const char Receiver_Command[]    PROGMEM = "Receiver.Command";
        const char Receiver_RawData[]    PROGMEM = "Receiver.RawData";
        const char Sensor_Button[]       PROGMEM = "Sensor.Button";
        const char Sensor_ColorHsv[]     PROGMEM = "Sensor.ColorHsv";
        const char Sensor_Humidity[]     PROGMEM = "Sensor.Humidity";
        const char Sensor_Level[]        PROGMEM = "Sensor.Level";
        const char Sensor_Luminance[]    PROGMEM = "Sensor.Luminance";
        const char Sensor_MotionDetect[] PROGMEM = "Sensor.MotionDetect";
        const char Sensor_Orientation[]  PROGMEM = "Sensor.Orientation";
        const char Sensor_Temperature[]  PROGMEM = "Sensor.Temperature";
        const char Status_Battery[]      PROGMEM = "Status.Battery";
        const char Status_ColorHsb[]     PROGMEM = "Status.ColorHsb";
        const char Status_Error[]        PROGMEM = "Status.Error";
        const char Status_IdleTime[]     PROGMEM = "Status.IdleTime";
        const char Status_Level[]        PROGMEM = "Status.Level";
        const char System_BytesFree[]    PROGMEM = "System.BytesFree";
        const char System_Status[]       PROGMEM = "System.Status";
    }
}


#endif //HOMEGENIE_MINI_IOEVENTPATHS_H
