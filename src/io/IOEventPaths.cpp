/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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

#include "IOEventPaths.h"

namespace IO {

    const char IOEventPaths::Sensor_RawData[] = "Sensor.RawData";
    const char IOEventPaths::Status_Level[] = "Status.Level";
    const char IOEventPaths::Sensor_Luminance[] = "Sensor.Luminance";
    const char IOEventPaths::Sensor_Temperature[] = "Sensor.Temperature";
    const char IOEventPaths::System_BytesFree[] = "System.BytesFree";

}