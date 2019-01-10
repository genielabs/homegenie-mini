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

#include "ReceiverConfig.h"

namespace IO { namespace X10 {

    ReceiverConfig::ReceiverConfig()
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
    ReceiverConfig::ReceiverConfig(uint8_t pin) : ReceiverConfig()
    {
        this->m_pin = pin;
    }
    ReceiverConfig::ReceiverConfig(
        uint8_t interrupt, uint8_t pin
    ) : ReceiverConfig(pin)
    {
        this->m_interrupt = interrupt;
    }
    ReceiverConfig::ReceiverConfig(
        uint16_t startBurstMin,
        uint16_t startBurstMax,
        uint16_t startBustRepeat,
        uint16_t bitZeroMin,
        uint16_t bitZeroMax,
        uint16_t bitOneMin,
        uint16_t bitOneMax)
    {
        this->m_startBustMin = startBurstMin;
        this->m_startBustMax = startBurstMax;
        this->m_startBustRepeat = startBustRepeat;

        this->m_bitZeroMin = bitZeroMin;
        this->m_bitZeroMax = bitZeroMax;

        this->m_bitOneMin = bitOneMin;
        this->m_bitOneMax = bitOneMax;
    }
    uint8_t ReceiverConfig::getPin() { return this->m_pin; };
    uint8_t ReceiverConfig::getInterrupt() { return this->m_interrupt; };
    uint16_t ReceiverConfig::getStartBustMin() { return this->m_startBustMin; };
    uint16_t ReceiverConfig::getStartBustMax() { return this->m_startBustMax; };
    uint16_t ReceiverConfig::getStartBustRepeat() { return this->m_startBustRepeat; };

    uint16_t ReceiverConfig::getBitZeroMin() { return this->m_bitZeroMin; };
    uint16_t ReceiverConfig::getBitZeroMax() { return this->m_bitZeroMax; };

    uint16_t ReceiverConfig::getBitOneMin() { return this->m_bitOneMin; };
    uint16_t ReceiverConfig::getBitOneMax() { return this->m_bitOneMax; };

}} // ns
