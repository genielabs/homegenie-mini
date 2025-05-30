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

#ifndef HOMEGENIE_MINI_X10_RF_TRANSMITTER_H_
#define HOMEGENIE_MINI_X10_RF_TRANSMITTER_H_

#include <io/Logger.h>

#include "X10RFTransmitterConfig.h"

namespace IO { namespace X10 {

    class X10RFTransmitter
    {
    public:
        X10RFTransmitter();
        explicit X10RFTransmitter(X10RFTransmitterConfig *configuration);
        void begin();
        void sendCommand(uint8_t *data, uint8_t size, uint8_t repeat, uint8_t sendDelay);
        X10RFTransmitterConfig* getConfiguration() { return configuration; }
    private:
        X10RFTransmitterConfig *configuration;
        void sendByte(uint8_t data);
        void sendBit(bool databit);
        void pulseHigh();
        void pulseLow();
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RF_TRANSMITTER_H_
