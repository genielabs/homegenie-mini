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

#include "RFReceiver.h"

namespace IO { namespace X10 {

    RFReceiver *receiverInstance = NULL;

    void receiverInstance_wrapper() {
        if (receiverInstance) receiverInstance->receive();
    }

    RFReceiver::RFReceiver() {
        receiverInstance = this;
    }

    RFReceiver::RFReceiver(RFReceiverConfig *configuration) : RFReceiver() {
        this->configuration = configuration;
    }

    //////////////////////////////
    /// Public
    //////////////////////////////

    void RFReceiver::begin() {
        Logger::info("|  - %s (PIN=%d INT=%d)", X10_RFRECEIVER_NS_PREFIX, configuration->getPin(),
                     configuration->getInterrupt());
        pinMode(configuration->getPin(), INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(configuration->getInterrupt()), receiverInstance_wrapper, RISING);
        Logger::info("|  ✔ %s", X10_RFRECEIVER_NS_PREFIX);
    }

    void RFReceiver::receive() {
        uint32_t currentTs = micros();
        uint32_t lengthUs = currentTs - riseUs;
        riseUs = currentTs;

        if (lengthUs >= configuration->getStartBustRepeat() && lengthUs <= configuration->getStartBustMax()) {
            receiveBuffer = 0;
            if (lengthUs >= configuration->getStartBustMin()) receivedCount = 1;
        } else if (receivedCount) {
            // Binary one received: add to buffer
            if (lengthUs >= configuration->getBitOneMin() && lengthUs <= configuration->getBitOneMax()) {
                receiveBuffer += 1LU << (receivedCount - 1);
            }
            // Invalid pulse length: stop receiving
            else if (lengthUs < configuration->getBitZeroMin() || lengthUs > configuration->getBitZeroMax()) {
                receivedCount = -1;
            }

            // Receive complete: verify message and fire callback
            if (receivedCount >= 32) {

// TODO: !!! IMPLEMENT 40 bit DATA DECODING

                byteBuffer[3] = Utility::reverseByte((receiveBuffer >> 24) & 0xFF);
                byteBuffer[2] = Utility::reverseByte((receiveBuffer >> 16) & 0xFF);
                byteBuffer[1] = Utility::reverseByte((receiveBuffer >> 8) & 0xFF);
                byteBuffer[0] = Utility::reverseByte(receiveBuffer & 0xFF);

                bool isSecurityCode = ((byteBuffer[1] ^ byteBuffer[0]) == 0x0F) && ((byteBuffer[3] ^ byteBuffer[2]) == 0xFF);
                bool isStandardCode = ((byteBuffer[1] & ~byteBuffer[0]) == byteBuffer[1] && (byteBuffer[3] & ~byteBuffer[2]) == byteBuffer[3]);


// TODO: !!! IMPLEMENT CHECKSUM VERIFICATION AND DISCARD MALFORMED MESSAGES


                if (isStandardCode || isSecurityCode) {
                    messageType = isStandardCode ? (uint8_t) 0x20 : (uint8_t) 0x29;
                    uint8_t data[] = { messageType, byteBuffer[0], byteBuffer[1], (byteBuffer[2]), (byteBuffer[3]) };
                    sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Receiver_RawData), data, IOEventDataType::Undefined);
                }

                receivedCount = -1;
                //receiveBuffer = 0;

            }

            receivedCount++;
        }
    }

}} // ns
