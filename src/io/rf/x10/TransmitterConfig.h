#ifndef HOMEGENIE_MINI_X10_TRANSMITTER_CONFIG_H_
#define HOMEGENIE_MINI_X10_TRANSMITTER_CONFIG_H_

#include "Arduino.h"

namespace X10
{
    class TransmitterConfig
    {
    private:
        uint8_t m_pin;
        uint8_t m_sendRepeat;
        uint16_t m_startBustLong;
        uint16_t m_startBustShort;
        uint16_t m_bitLong;
        uint16_t m_bitShort;
        uint16_t m_packetGap;
    public:
        TransmitterConfig();
        TransmitterConfig(uint8_t pin);
        uint8_t getPin();
        uint8_t getSendRepeat();
        uint16_t getStartBustLong();
        uint16_t getStartBustShort();
        uint16_t getBitLong();
        uint16_t getBitShort();
        uint16_t getPacketGap();
    };
}

#endif // HOMEGENIE_MINI_X10_TRANSMITTER_CONFIG_H_
