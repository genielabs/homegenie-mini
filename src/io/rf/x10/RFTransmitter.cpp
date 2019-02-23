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

#include "RFTransmitter.h"

namespace IO { namespace X10 {

    /*

     (source: http://linuxha.com/athome/common/x10.rf.html)

     Format of RF envelope

        The carrier is at a nominal frequency of 300Mhz or 433Mhz.  The envelope of the
        carrier is sketched below.  Time progresses from left to right.

        ---------------        ---------       ---------                    -
        |             |        |       |       |       |                    |
        |   8msec     | 4msec  | 0.4ms | 0.7ms | 0.4ms | 0.7ms    1.1 msec  |
        -             ----------       ---------       --------+-------------
             RF preample       + sampled '1'   +  sampled '1'  + sampled '0'

        The data format is return-to-zero.  A '1' is sampled when the envelope
        goes high for 0.4msec and then returns low for 0.7msec.  Thus with the
        above waveform, the sampled data (left bit below is received first):

        110.......

        Note that the RF preamble is not part of the sampled data, and that I
        make the distinction between the received envelope (sketched above)
        and the SAMPLED STREAM (the 110.. above).  This sampled stream is
        converted to a DECODED STREAM and then converted to an actual
        X-10/security command using the procedure explained below.

    */

    RFTransmitter::RFTransmitter() {

    }
    RFTransmitter::RFTransmitter(RFTransmitterConfig *configuration) : RFTransmitter() {
        this->configuration = configuration;
    }

    void RFTransmitter::begin() {
        Logger::info("|  - IO::X10::RFTransmitter (PIN=%d)", configuration->getPin());
        pinMode(configuration->getPin(), OUTPUT);
        Logger::info("|  ✔ IO::X10::RFTransmitter");
    }

    void RFTransmitter::sendCommand(uint8_t *data, uint8_t size, uint8_t repeat) {
        for (int i = 0; i < (repeat > 0 ? repeat : configuration->getSendRepeat()); i++) {
            pulseHigh();
            delayMicroseconds(configuration->getStartBustLong());
            pulseLow();
            delayMicroseconds(configuration->getStartBustShort());
            for (int i = 0; i < size; i++) {
                sendByte(data[i]);
            }
            sendBit(true);
            delayMicroseconds(configuration->getPacketGap());
        }
    }

    void RFTransmitter::sendByte(uint8_t data) {
        for (int i = 7; i >= 0; i--) { // send bits from byte
            sendBit(bitRead(data, i) == 1);
        }
    }

    void RFTransmitter::sendBit(bool databit) {
        pulseHigh();
        delayMicroseconds(configuration->getBitShort());
        pulseLow();
        delayMicroseconds(configuration->getBitShort());
        if (databit) delayMicroseconds(configuration->getBitLong());
    }

    void RFTransmitter::pulseHigh() {
        digitalWrite(configuration->getPin(), HIGH);
    }

    void RFTransmitter::pulseLow() {
        digitalWrite(configuration->getPin(), LOW);
    }

}} // ns
