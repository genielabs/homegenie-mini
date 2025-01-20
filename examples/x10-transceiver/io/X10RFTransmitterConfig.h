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
 * - 2019-01-10 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_X10_RF_TRANSMITTER_CONFIG_H_
#define HOMEGENIE_MINI_X10_RF_TRANSMITTER_CONFIG_H_

#include <Config.h>

#include "../configuration.h"

namespace IO { namespace X10 {

    class X10RFTransmitterConfig
    {
    private:
        uint8_t pin;
        uint8_t sendRepeat;
        uint16_t startBustLong;
        uint16_t startBustShort;
        uint16_t bitLong;
        uint16_t bitShort;
        uint16_t packetGap;
    public:
        X10RFTransmitterConfig();
        explicit X10RFTransmitterConfig(uint8_t pin);
        uint8_t getPin();
        uint8_t getSendRepeat();
        uint16_t getStartBustLong();
        uint16_t getStartBustShort();
        uint16_t getBitLong();
        uint16_t getBitShort();
        uint16_t getPacketGap();
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RF_TRANSMITTER_CONFIG_H_
