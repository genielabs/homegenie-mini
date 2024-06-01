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

#include "IRReceiver.h"

namespace IO { namespace IR {

    IRReceiver *receiverInstance = NULL;

    IRReceiver::IRReceiver() {
        receiverInstance = this;
    }

    IRReceiver::IRReceiver(IRReceiverConfig *configuration) : IRReceiver() {
        this->configuration = configuration;
    }

    //////////////////////////////
    /// Public
    //////////////////////////////

    void IRReceiver::begin() {
        Logger::info("|  - %s (PIN=%d INT=%d)", IR_IRRECEIVER_NS_PREFIX,
                     configuration->getPin(),
                     configuration->getInterrupt());

        irReceiver = new IRrecv(configuration->getPin(), 1024, 50, false, 1);
        //irReceiver->setTolerance(10);
        enabled(true);

        Logger::info("|  ✔ %s", IR_IRRECEIVER_NS_PREFIX);
    }

    bool IRReceiver::enabled() {
        return isEnabled;
    }
    void IRReceiver::enabled(bool enabled) {
        if (enabled && !isEnabled) irReceiver->enableIRIn();
        else if (!enabled && isEnabled) irReceiver->disableIRIn();
        isEnabled = enabled;
    }

    void IRReceiver::loop() {

        if (isEnabled && irReceiver != nullptr && irReceiver->decode(&results)) {  // We have captured something.

            decode_type_t protocol = results.decode_type;
            uint16_t size = results.bits;

            String sProtocol = Utility::byteToHex((int16_t)protocol);
            String sSize = Utility::byteToHex(size);
            String sData = "";

            if (protocol == decode_type_t::UNKNOWN) {
                size = getCorrectedRawLength(&results);
                sSize = Utility::byteToHex(size);
                uint16_t* data = resultToRawArray(&results);
                sData = Utility::getByteString(data, size);
                // Deallocate the memory allocated by resultToRawArray().
                delete[] data;
            } else if (hasACState(protocol)) {
                sSize = Utility::byteToHex((uint16_t )(size / 8));
                sData = Utility::getByteString(results.state, size / 8);
            } else {  // simple message protocol (<= 64 bits)
                sData = String(results.value, HEX);
            }

            if (sData != "ffffffffffffffff") {
                //Serial.println(resultToSourceCode(&results));
                String stringData = sProtocol + sSize + sData;
                stringData.toUpperCase();
                sendEvent(IOEventPaths::Receiver_RawData, &stringData, IOEventDataType::Text);
            }

            irReceiver->resume();
        }

    }

}} // ns
