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
 * - 2020-01-18 Initial release
 *
 */

/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/

#include "RFTransmitter.h"

namespace IO { namespace RCS {

    const byte protocol = 1; // <--- See RCSwitch documentation

    RFTransmitter::RFTransmitter() {
    }

    RFTransmitter::RFTransmitter(RFTransmitterConfig *configuration) : RFTransmitter() {
        this->configuration = configuration;
    }

    void RFTransmitter::begin() {
        Logger::info("|  - IO::RemoteControl::RFTransmitter (PIN=%d)", configuration->getPin());
        RF.disableReceive();
        RF.enableTransmit(configuration->getPin());   // Set RF transmit pin
        RF.setProtocol(protocol);                     // Set RF protocol
        Logger::info("|  ✔ IO::RemoteControl::RFTransmitter");
    }

    void RFTransmitter::sendCommand(long command, unsigned short bitLength, unsigned short repeat, unsigned short repeat_delay) {
// TODO: rewrite ASYNC (move to loop() SEE ir-transceiver/io/IRTransmitter.cpp)
        for (int i = 0; i < repeat; i++) {
            RF.send(command, bitLength);
            delay(repeat_delay);
        }
    }

}}
