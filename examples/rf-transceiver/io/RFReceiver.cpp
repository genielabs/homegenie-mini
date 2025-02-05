/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
 */

#include "RFReceiver.h"

namespace IO { namespace RCS {

    RFReceiver::RFReceiver() {
        setLoopInterval(1);
    }

    RFReceiver::RFReceiver(RFReceiverConfig *configuration) : RFReceiver() {
        this->configuration = configuration;
    }


    //////////////////////////////
    /// Public
    //////////////////////////////

    void RFReceiver::begin() {
        Logger::info("|  - %s (PIN=%d INT=%d)", RF_RFRECEIVER_NS_PREFIX,
                     configuration->getPin(),
                     configuration->getInterrupt());

        //RF.setPulseLength(7000);
        //RF.setReceiveTolerance(100);

        RF.disableTransmit();
        RF.enableReceive(configuration->getPin());

        Logger::info("|  ✔ %s", RF_RFRECEIVER_NS_PREFIX);
    }

    void RFReceiver::loop() {

        if (RF.available()) {  // We have captured something.
//            Serial.print("Received ");
//            Serial.print(RF.getReceivedValue());
//            Serial.print(" / ");
//            Serial.print(RF.getReceivedBitlength());
//            Serial.print("bit ");
//            Serial.print("Protocol: ");
//            Serial.println(RF.getReceivedProtocol());

            auto value = RF.getReceivedValue();
            auto hexData = Utility::byteToHex((uint32_t)value);
            hexData.toUpperCase();

            Utility::getBytes(hexData, eventData);
            eventData[0] = (RF.getReceivedBitlength() << 3) + RF.getReceivedProtocol();

            sendEvent(IOEventPaths::Receiver_RawData, eventData, IOEventDataType::Undefined);

            RF.resetAvailable();
        }

    }

    void RFReceiver::handleInterrupt() {
        if (enableReceive) {
            RF.handleInterrupt();
        }
    }

    void RFReceiver::enable() {
        enableReceive = true;
    }

    void RFReceiver::disable() {
        enableReceive = false;
    }

}} // ns
