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

#ifndef HOMEGENIE_MINI_X10_RFRECEIVER_H_
#define HOMEGENIE_MINI_X10_RFRECEIVER_H_

#include "Arduino.h"

#include <io/Logger.h>
#include <io/IOEvent.h>
#include <io/IOEventPaths.h>
#include <io/IOEventDomains.h>

#include "RfReceiverConfig.h"

#define X10_RFRECEIVER_NS_PREFIX                  "IO::X10::RfReceiver"

namespace IO { namespace X10 {

    class RfReceiver : public IIOEventSender {
    public:
        RfReceiver();
        RfReceiver(RfReceiverConfig *);

        void begin();
        void receive();

        void enable();
        void disable();

        bool isEnabled();

    private:
        bool enabled;
        RfReceiverConfig *configuration;
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

#endif // HOMEGENIE_MINI_X10_RFRECEIVER_H_
