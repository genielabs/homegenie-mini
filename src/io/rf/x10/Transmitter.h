#ifndef HOMEGENIE_MINI_X10_TRANSMITTER_H_
#define HOMEGENIE_MINI_X10_TRANSMITTER_H_

#include "Arduino.h"
#include "TransmitterConfig.h"

namespace X10
{

    class Transmitter
    {
    public:
        Transmitter();
        Transmitter(TransmitterConfig *configuration);
        void begin();
        void sendCommand(uint8_t *data, uint8_t size);
    private:
        TransmitterConfig *configuration;
        void sendByte(uint8_t data);
        void sendBit(bool databit);
        void pulseHigh();
        void pulseLow();
    };

}

#endif // HOMEGENIE_MINI_X10_TRANSMITTER_H_
