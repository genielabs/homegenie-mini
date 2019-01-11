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
 * - 2019-10-01 Initial release
 *
 */

#include "RfTransmitterConfig.h"

namespace IO { namespace X10 {

    RfTransmitterConfig::RfTransmitterConfig()
    {
        this->m_pin = 4;
        this->m_sendRepeat = 4;

        this->m_startBustLong = 9000;
        this->m_startBustShort = 4500;

        this->m_bitLong = 1110;
        this->m_bitShort = 555;

        this->m_packetGap = 29000;
    }
    RfTransmitterConfig::RfTransmitterConfig(uint8_t pin) : RfTransmitterConfig()
    {
        this->m_pin = pin;
    }
    uint8_t RfTransmitterConfig::getPin() { return this->m_pin; };
    uint8_t RfTransmitterConfig::getSendRepeat() { return this->m_sendRepeat; };
    uint16_t RfTransmitterConfig::getStartBustLong() { return this->m_startBustLong; };
    uint16_t RfTransmitterConfig::getStartBustShort() { return this->m_startBustShort; };

    uint16_t RfTransmitterConfig::getBitLong() { return this->m_bitLong; };
    uint16_t RfTransmitterConfig::getBitShort() { return this->m_bitShort; };

    uint16_t RfTransmitterConfig::getPacketGap() { return this->m_packetGap; };

}} // ns
