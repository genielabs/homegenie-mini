#include "TransmitterConfig.h"

namespace X10
{
    TransmitterConfig::TransmitterConfig()
    {
        this->m_pin = 4;
        this->m_sendRepeat = 4;

        this->m_startBustLong = 9000;
        this->m_startBustShort = 4500;

        this->m_bitLong = 1110;
        this->m_bitShort = 555;

        this->m_packetGap = 29000;
    }
    TransmitterConfig::TransmitterConfig(uint8_t pin) : TransmitterConfig()
    {
        this->m_pin = pin;
    }
    uint8_t TransmitterConfig::getPin() { return this->m_pin; };
    uint8_t TransmitterConfig::getSendRepeat() { return this->m_sendRepeat; };
    uint16_t TransmitterConfig::getStartBustLong() { return this->m_startBustLong; };
    uint16_t TransmitterConfig::getStartBustShort() { return this->m_startBustShort; };

    uint16_t TransmitterConfig::getBitLong() { return this->m_bitLong; };
    uint16_t TransmitterConfig::getBitShort() { return this->m_bitShort; };

    uint16_t TransmitterConfig::getPacketGap() { return this->m_packetGap; };
}
