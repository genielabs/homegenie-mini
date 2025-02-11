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

#include "RFReceiverConfig.h"

namespace IO { namespace RCS {

    RFReceiverConfig::RFReceiverConfig()
    {
        pin = interrupt = String(CONFIG_RCSwitchReceiverPin).toInt();
    }
    RFReceiverConfig::RFReceiverConfig(uint8_t pin) : RFReceiverConfig()
    {
        this->pin = pin;
    }
    RFReceiverConfig::RFReceiverConfig(
        uint8_t interrupt, uint8_t pin
    ) : RFReceiverConfig(pin)
    {
        this->interrupt = interrupt;
    }

    uint8_t RFReceiverConfig::getPin() { return pin; };
    uint8_t RFReceiverConfig::getInterrupt() { return interrupt; }

}} // ns
