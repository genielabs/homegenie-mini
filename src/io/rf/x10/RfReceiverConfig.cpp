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

#include "RfReceiverConfig.h"

namespace IO { namespace X10 {

    RfReceiverConfig::RfReceiverConfig()
    {
        this->m_pin = this->m_interrupt = 5;

        this->m_startBustMin = 8000;
        this->m_startBustMax = 16000;
        this->m_startBustRepeat = 7000;

        this->m_bitZeroMin = 800;
        this->m_bitZeroMax = 1400;

        this->m_bitOneMin = 2000;
        this->m_bitOneMax = 2400;
    }
    RfReceiverConfig::RfReceiverConfig(uint8_t pin) : RfReceiverConfig()
    {
        this->m_pin = pin;
    }
    RfReceiverConfig::RfReceiverConfig(
        uint8_t interrupt, uint8_t pin
    ) : RfReceiverConfig(pin)
    {
        this->m_interrupt = interrupt;
    }
    uint8_t RfReceiverConfig::getPin() { return this->m_pin; };
    uint8_t RfReceiverConfig::getInterrupt() { return this->m_interrupt; };
    uint16_t RfReceiverConfig::getStartBustMin() { return this->m_startBustMin; };
    uint16_t RfReceiverConfig::getStartBustMax() { return this->m_startBustMax; };
    uint16_t RfReceiverConfig::getStartBustRepeat() { return this->m_startBustRepeat; };

    uint16_t RfReceiverConfig::getBitZeroMin() { return this->m_bitZeroMin; };
    uint16_t RfReceiverConfig::getBitZeroMax() { return this->m_bitZeroMax; };

    uint16_t RfReceiverConfig::getBitOneMin() { return this->m_bitOneMin; };
    uint16_t RfReceiverConfig::getBitOneMax() { return this->m_bitOneMax; };

}} // ns
