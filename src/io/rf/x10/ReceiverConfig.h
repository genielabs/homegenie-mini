#ifndef HOMEGENIE_MINI_X10_RECEIVER_CONFIG_H_
#define HOMEGENIE_MINI_X10_RECEIVER_CONFIG_H_

#include "Arduino.h"

namespace X10
{
    class ReceiverConfig
    {
    private:
        uint8_t m_interrupt;
        uint8_t m_pin;
        uint16_t m_startBustMin;
        uint16_t m_startBustMax;
        uint16_t m_startBustRepeat;

        uint16_t m_bitZeroMin;
        uint16_t m_bitZeroMax;

        uint16_t m_bitOneMin;
        uint16_t m_bitOneMax;
    public:
        ReceiverConfig();
        ReceiverConfig(uint8_t pin);
        ReceiverConfig(uint8_t interrupt, uint8_t pin);
        ReceiverConfig(
            uint16_t startBurstMin, uint16_t startBurstMax, uint16_t startBustRepeat,
            uint16_t bitZeroMin, uint16_t bitZeroMax,
            uint16_t bitOneMin, uint16_t bitOneMax
        );
        uint8_t getPin();
        uint8_t getInterrupt();
        uint16_t getStartBustMin();
        uint16_t getStartBustMax();
        uint16_t getStartBustRepeat();

        uint16_t getBitZeroMin();
        uint16_t getBitZeroMax();

        uint16_t getBitOneMin();
        uint16_t getBitOneMax();
    };
    /*
    class TransmitterConfig
    {
    public:
        int StartBustLong;
        int StartBustShort;
        int BitLong;
        int BitShort;
        int PacketGap;
    };
    */
}

#endif // HOMEGENIE_MINI_X10_RECEIVER_CONFIG_H_
