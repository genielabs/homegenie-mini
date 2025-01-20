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

#include "X10RFReceiver.h"

namespace IO { namespace X10 {

    X10RFReceiver *receiverInstance = nullptr;
    std::function<void()> customInterruptHandler = nullptr;
    bool isPinLow = false;
    uint8_t dataPin = 0;

    void ICACHE_RAM_ATTR receiverInstance_wrapper() {
        auto pv = digitalRead(dataPin);
        bool rising = isPinLow && pv == HIGH;
        isPinLow = pv == LOW;
        if (receiverInstance && rising) {
            receiverInstance->receive();
        }
        // Call custom interrupt callback if set
        if (customInterruptHandler) {
            customInterruptHandler();
        }
    }

    X10RFReceiver::X10RFReceiver() {
        setLoopInterval(50);
        receiverInstance = this;
    }

    X10RFReceiver::X10RFReceiver(X10RFReceiverConfig *configuration) : X10RFReceiver() {
        this->configuration = configuration;
    }

    //////////////////////////////
    /// Public
    //////////////////////////////

    void X10RFReceiver::begin() {
        Logger::info("|  - %s (PIN=%d INT=%d)", X10_RFRECEIVER_NS_PREFIX, configuration->getPin(),
                     configuration->getInterrupt());
        dataPin = configuration->getPin();
        pinMode(dataPin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(configuration->getInterrupt()), receiverInstance_wrapper, CHANGE);
        Logger::info("|  ✔ %s", X10_RFRECEIVER_NS_PREFIX);
    }

    void X10RFReceiver::loop() {
        if (millis() < disabledToMs) {
            eventReady = false;
            return;
        }
        if (eventReady) {
            eventReady = false;
            sendEvent(IOEventPaths::Receiver_RawData, eventData, IOEventDataType::Undefined);
        }
    }

    void X10RFReceiver::disableMs(uint32_t ms) {
        disabledToMs = millis() + ms;
    }

    void X10RFReceiver::receive() {
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
                    eventData[0] = messageType;
                    eventData[1] = byteBuffer[0];
                    eventData[2] = byteBuffer[1];
                    eventData[3] = byteBuffer[2];
                    eventData[4] = byteBuffer[3];
                    eventReady = true;
                }

                receivedCount = -1;
                //receiveBuffer = 0;

            }

            receivedCount++;
        }
    }

    void X10RFReceiver::addInterruptHandler(std::function<void()> handler) {
        customInterruptHandler = std::move(handler);
    }

}} // ns
