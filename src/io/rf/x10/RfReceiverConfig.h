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

#ifndef HOMEGENIE_MINI_X10_RF_RECEIVER_CONFIG_H_
#define HOMEGENIE_MINI_X10_RF_RECEIVER_CONFIG_H_

#include "Arduino.h"

namespace IO { namespace X10 {
    /**
     * Decodes X10 RF messages
     */
    class RfReceiverConfig
    {
    public:
        RfReceiverConfig();
        RfReceiverConfig(uint8_t pin);
        RfReceiverConfig(uint8_t interrupt, uint8_t pin);
        uint8_t getPin();
        uint8_t getInterrupt();
        uint16_t getStartBustMin();
        uint16_t getStartBustMax();
        uint16_t getStartBustRepeat();

        uint16_t getBitZeroMin();
        uint16_t getBitZeroMax();

        uint16_t getBitOneMin();
        uint16_t getBitOneMax();
    private:
        uint8_t m_interrupt;
        uint8_t m_pin;
        uint16_t m_startBustMin;
        uint16_t m_startBustMax;
        uint16_t m_startBustRepeat;

        uint16_t m_bitZeroMin;
        uint16_t m_bitZeroMax;

        uint16_t m_bitOneMin;
        uint16_t m_bitOneMax;
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RF_RECEIVER_CONFIG_H_
