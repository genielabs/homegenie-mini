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

#ifndef HOMEGENIE_MINI_X10_RFRECEIVER_H_
#define HOMEGENIE_MINI_X10_RFRECEIVER_H_

#include <HomeGenie.h>
#include <Utility.h>

#include "RFReceiverConfig.h"

#define X10_RFRECEIVER_NS_PREFIX                  "IO::X10::RfReceiver"

namespace IO { namespace X10 {

    class RFReceiver : public IIOEventSender {
    public:
        RFReceiver();
        RFReceiver(RFReceiverConfig *);

        void begin();
        void receive();

    private:
        String domain = IOEventDomains::HomeAutomation_X10;
        String address = CONFIG_X10RF_MODULE_ADDRESS;
        RFReceiverConfig *configuration;
        // 32-bit RF message decoding
        volatile uint8_t messageType = 0x00;
        volatile uint8_t byteBuffer[4];
        volatile uint32_t riseUs;
        volatile int8_t receivedCount;
        volatile uint32_t receiveBuffer;
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RFRECEIVER_H_
