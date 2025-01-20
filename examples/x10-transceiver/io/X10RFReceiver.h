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

#include "X10RFReceiverConfig.h"

#define X10_RFRECEIVER_NS_PREFIX                  "IO::X10::RfReceiver"

namespace IO { namespace X10 {

    class X10RFReceiver : public Task, public IIOEventSender {
    public:
        X10RFReceiver();
        explicit X10RFReceiver(X10RFReceiverConfig *);

        void loop() override;
        void begin() override;
        void receive();
        static void addInterruptHandler(std::function<void(void)> handler);
        void disableMs(uint32_t ms);

    private:
        X10RFReceiverConfig *configuration;
        // 32-bit RF message decoding
        volatile uint8_t messageType = 0x00;
        volatile uint8_t byteBuffer[4];
        volatile uint32_t riseUs = 0;
        volatile int8_t receivedCount = 0;
        volatile uint32_t receiveBuffer = 0;
        // event data
        uint8_t eventData[5];
        bool eventReady = false;
        uint32_t disabledToMs = 0;
    };

}} // ns

#endif // HOMEGENIE_MINI_X10_RFRECEIVER_H_
