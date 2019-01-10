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

#ifndef HOMEGENIE_MINI_X10_RECEIVER_H_
#define HOMEGENIE_MINI_X10_RECEIVER_H_

#include "Arduino.h"
#include "ReceiverConfig.h"
#include "io/Logger.h"

namespace IO { namespace X10 {

    class Receiver {
    public:
        class X10RfDataReceivedCallback {
        public:
            virtual void onX10RfDataReceived(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
        };

        Receiver();

        Receiver(ReceiverConfig *configuration, X10RfDataReceivedCallback *rfReceiveCallback);

        void begin();

        void receive();

        void enable();

        void disable();

        bool isEnabled();

    private:
        bool enabled;
        ReceiverConfig *configuration;
        X10RfDataReceivedCallback *rfReceiveCallback;
        // 32-bit RF message decoding
        uint32_t riseUs;
        int8_t receivedCount;
        uint32_t receiveBuffer;

        // TODO: move to an utility class (maybe static)
        // Utility methods
        uint32_t reverseBits(uint32_t n);
        uint8_t reverseByte(uint8_t n);
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RECEIVER_H_
