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

#include "RfReceiverConfig.h"

namespace IO { namespace X10 {

    RfReceiverConfig::RfReceiverConfig()
    {
        pin = interrupt = 5;

        startBustMin = 8000;
        startBustMax = 16000;
        startBustRepeat = 7000;

        bitZeroMin = 800;
        bitZeroMax = 1400;

        bitOneMin = 2000;
        bitOneMax = 2400;
    }
    RfReceiverConfig::RfReceiverConfig(uint8_t pin) : RfReceiverConfig()
    {
        this->pin = pin;
    }
    RfReceiverConfig::RfReceiverConfig(
        uint8_t interrupt, uint8_t pin
    ) : RfReceiverConfig(pin)
    {
        this->interrupt = interrupt;
    }
    uint8_t RfReceiverConfig::getPin() { return pin; };
    uint8_t RfReceiverConfig::getInterrupt() { return interrupt; };
    uint16_t RfReceiverConfig::getStartBustMin() { return startBustMin; };
    uint16_t RfReceiverConfig::getStartBustMax() { return startBustMax; };
    uint16_t RfReceiverConfig::getStartBustRepeat() { return startBustRepeat; };

    uint16_t RfReceiverConfig::getBitZeroMin() { return bitZeroMin; };
    uint16_t RfReceiverConfig::getBitZeroMax() { return bitZeroMax; };

    uint16_t RfReceiverConfig::getBitOneMin() { return bitOneMin; };
    uint16_t RfReceiverConfig::getBitOneMax() { return bitOneMax; };

}} // ns
