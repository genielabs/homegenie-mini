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

#include "IRReceiverConfig.h"

namespace IO { namespace IR {

    IRReceiverConfig::IRReceiverConfig()
    {
        pin = interrupt = CONFIG_IRReceiverPin; // 5
    }
    IRReceiverConfig::IRReceiverConfig(uint8_t pin) : IRReceiverConfig()
    {
        this->pin = pin;
    }
    IRReceiverConfig::IRReceiverConfig(
        uint8_t interrupt, uint8_t pin
    ) : IRReceiverConfig(pin)
    {
        this->interrupt = interrupt;
    }
    uint8_t IRReceiverConfig::getPin() { return pin; };
    uint8_t IRReceiverConfig::getInterrupt() { return interrupt; };

}} // ns
