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
 * - 2020-01-18 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_RCS_RFTRANSMITTERCONFIG_H
#define HOMEGENIE_MINI_RCS_RFTRANSMITTERCONFIG_H

#include <HomeGenie.h>

#include "../configuration.h"

namespace IO { namespace RCS {
    /**
     * Encodes RCS RF messages
     */
    class RFTransmitterConfig
    {
    public:
        RFTransmitterConfig();
        RFTransmitterConfig(uint8_t pin);
        uint8_t getPin();
    private:
        uint8_t pin;
    };
}} // ns

#endif //HOMEGENIE_MINI_RCS_RFTRANSMITTERCONFIG_H
