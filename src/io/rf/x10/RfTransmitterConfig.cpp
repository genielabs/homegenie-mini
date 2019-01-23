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
 * - 2019-01-10 Initial release
 *
 */

#include "RfTransmitterConfig.h"

namespace IO { namespace X10 {

    RfTransmitterConfig::RfTransmitterConfig()
    {
        pin = D2; // 4
        sendRepeat = 3;

        startBustLong = 9000;
        startBustShort = 4500;

        bitLong = 1110;
        bitShort = 555;

        packetGap = 29000;
    }
    RfTransmitterConfig::RfTransmitterConfig(uint8_t pin) : RfTransmitterConfig()
    {
        this->pin = pin;
    }
    uint8_t RfTransmitterConfig::getPin() { return this->pin; };
    uint8_t RfTransmitterConfig::getSendRepeat() { return this->sendRepeat; };
    uint16_t RfTransmitterConfig::getStartBustLong() { return this->startBustLong; };
    uint16_t RfTransmitterConfig::getStartBustShort() { return this->startBustShort; };

    uint16_t RfTransmitterConfig::getBitLong() { return this->bitLong; };
    uint16_t RfTransmitterConfig::getBitShort() { return this->bitShort; };

    uint16_t RfTransmitterConfig::getPacketGap() { return this->packetGap; };

}} // ns
