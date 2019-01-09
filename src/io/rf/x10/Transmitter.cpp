#include "Transmitter.h"
#include "TransmitterConfig.h"

#include <stdlib.h>
#include "Arduino.h"

namespace X10 {

    /*

     (source: http://linuxha.com/athome/common/x10.rf.html)

     Format of RF envelope

        The carrier is at a nominal frequency of 300 Mhz.  The envelope of the
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

    Transmitter::Transmitter() {

    }
    Transmitter::Transmitter(TransmitterConfig *configuration) : Transmitter() {
        this->configuration = configuration;
    }

    void Transmitter::begin() {
        pinMode(configuration->getPin(), OUTPUT);
        Serial.println("X10::Transmitter started.");
    }

    void Transmitter::sendCommand(uint8_t *data, uint8_t size) {
        for (int i = 0; i < configuration->getSendRepeat(); i++) {
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

    void Transmitter::sendByte(uint8_t data) {
        //Serial.println("\n");
        for (int i = 7; i >= 0; i--) { // send bits from byte
            sendBit(bitRead(data, i) == 1);
            //Serial.print(bitRead(data,i));
        }
    }

    void Transmitter::sendBit(bool databit) {
        pulseHigh();
        delayMicroseconds(configuration->getBitShort());
        pulseLow();
        delayMicroseconds(configuration->getBitShort());
        if (databit) delayMicroseconds(configuration->getBitLong());
    }

    void Transmitter::pulseHigh() {
        digitalWrite(configuration->getPin(), HIGH);
    }

    void Transmitter::pulseLow() {
        digitalWrite(configuration->getPin(), LOW);
    }
}
