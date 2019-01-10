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

#include "Receiver.h"

namespace IO { namespace X10 {

    Receiver *receiverInstance = NULL;

    void receiverInstance_wrapper() {
        if (receiverInstance) receiverInstance->receive();
    }

    Receiver::Receiver() {
        receiverInstance = this;
    }

    Receiver::Receiver(ReceiverConfig *configuration, X10RfDataReceivedCallback *rfReceiveCallback) : Receiver() {
        this->configuration = configuration;
        this->rfReceiveCallback = rfReceiveCallback;
    }

    //////////////////////////////
    /// Public
    //////////////////////////////

    void Receiver::begin() {
        enabled = true;
        if (rfReceiveCallback) {
            pinMode(configuration->getPin(), INPUT);
            attachInterrupt(configuration->getInterrupt(), receiverInstance_wrapper, RISING);
        }
        Logger::info("|  IO::X10::Receiver started.");
    }

    void Receiver::enable() {
        enabled = true;
    }

    void Receiver::disable() {
        enabled = false;
    }

    bool Receiver::isEnabled() {
        return enabled;
    }

    uint8_t messageType = 0x00;
    uint8_t byteBuffer[4];

    void Receiver::receive() {
        uint32_t lengthUs = micros() - riseUs;
        riseUs = micros();

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

            // Receive complete: parse message
            if (receivedCount == 32) {

                // TODO: reverse

                byteBuffer[3] = reverseByte((receiveBuffer >> 24) & 0xFF);
                byteBuffer[2] = reverseByte((receiveBuffer >> 16) & 0xFF);
                byteBuffer[1] = reverseByte((receiveBuffer >> 8) & 0xFF);
                byteBuffer[0] = reverseByte(receiveBuffer & 0xFF);

                /*
                b1 = reverseByte(b1);
                b3 = reverseByte(b3);
                */
                bool isSecurityCode = ((byteBuffer[1] ^ byteBuffer[0]) == 0x0F) && ((byteBuffer[3] ^ byteBuffer[2]) == 0xFF);
                bool isStandardCode = ((byteBuffer[1] & ~byteBuffer[0]) == byteBuffer[1] && (byteBuffer[3] & ~byteBuffer[2]) == byteBuffer[3]);

                if (isStandardCode || isSecurityCode) {
                    messageType = isStandardCode ? (uint8_t) 0x20 : (uint8_t) 0x29;
                    rfReceiveCallback->onX10RfDataReceived(messageType, byteBuffer[0], byteBuffer[1], (byteBuffer[2]), (byteBuffer[3]));
                }

                receivedCount = -1;
                //receiveBuffer = 0;

            }

            receivedCount++;
        }
    }

    // TODO: move to an utility class (maybe static)

    uint8_t Receiver::reverseByte(uint8_t b) {
        b = (b & (uint8_t) 0xF0) >> (uint8_t) 4 | (b & (uint8_t) 0x0F) << 4;
        b = (b & (uint8_t) 0xCC) >> 2 | (b & (uint8_t) 0x33) << 2;
        b = (b & (uint8_t) 0xAA) >> 1 | (b & (uint8_t) 0x55) << 1;
        return b;
    }

    uint32_t Receiver::reverseBits(uint32_t n) {
        uint32_t x = 0;
        for (auto i = 31; n;) {
            x |= (n & 1) << i;
            n >>= 1;
            --i;
        }
        return x;
    }

}} // ns
