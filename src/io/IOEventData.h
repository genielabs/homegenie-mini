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
 */

#ifndef HOMEGENIE_MINI_IOEVENTDATA_H
#define HOMEGENIE_MINI_IOEVENTDATA_H

namespace IO {

    enum IOEventDataType {
        Undefined = 0,
        Text,
        Binary,
        Number,
        Float,
        UnsignedNumber,
        SensorLight,
        SensorTemperature,
        SensorHumidity,
        SensorColorHsv
    };

    class IOEventBinaryData {
    public:
        size_t length;
        void* data;
        size_t type_size;
    };
}

#endif //HOMEGENIE_MINI_IOEVENTDATA_H
