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
 */

#include "RFReceiver.h"

namespace IO { namespace RCS {

    RFReceiver *receiverInstance = NULL;

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

            Serial.print("Received ");
            Serial.print(RF.getReceivedValue());
            Serial.print(" / ");
            Serial.print(RF.getReceivedBitlength());
            Serial.print("bit ");
            Serial.print("Protocol: ");
            Serial.println(RF.getReceivedProtocol());


            // TODO:
            // TODO:
            //    sendEvent((const uint8_t*)(IOEventPaths::Receiver_RawData), eventData, IOEventDataType::Undefined);
            // TODO: auto eventData = RF.getReceivedValue();


            RF.resetAvailable();

        }

    }

}} // ns
