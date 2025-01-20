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

#include "X10RFTransmitterConfig.h"

namespace IO { namespace X10 {

    X10RFTransmitterConfig::X10RFTransmitterConfig()
    {
        pin = CONFIG_X10RFTransmitterPin; // 4
        sendRepeat = 3;

        startBustLong = 9000;
        startBustShort = 4500;

        bitLong = 1110;
        bitShort = 555;

        packetGap = 29000;
    }
    X10RFTransmitterConfig::X10RFTransmitterConfig(uint8_t pin) : X10RFTransmitterConfig()
    {
        this->pin = pin;
    }
    uint8_t X10RFTransmitterConfig::getPin() { return pin; };
    uint8_t X10RFTransmitterConfig::getSendRepeat() { return sendRepeat; };
    uint16_t X10RFTransmitterConfig::getStartBustLong() { return startBustLong; };
    uint16_t X10RFTransmitterConfig::getStartBustShort() { return startBustShort; };

    uint16_t X10RFTransmitterConfig::getBitLong() { return bitLong; };
    uint16_t X10RFTransmitterConfig::getBitShort() { return bitShort; };

    uint16_t X10RFTransmitterConfig::getPacketGap() { return packetGap; };

}} // ns
