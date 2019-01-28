//
// Created by gene on 28/01/19.
//

#include "Utility.h"

// TODO: move to an utility class (maybe static)
/// Convert byte to hex string taking care of leading-zero
/// \param b
void Utility::getBytes(const String &rawBytes, uint8_t *data) {
    uint len = rawBytes.length();
    char msg[len+1]; rawBytes.toCharArray(msg, len+1, 0);
    char tmp[3] = "00";
    len = (len / 2);
    for (uint i = 0; i < len; i++)
    {
        tmp[0] = msg[i * 2];
        tmp[1] = msg[(i * 2) + 1];
        data[i] = strtol(tmp, NULL, 16);
    }
}
String Utility::byteToHex(byte b) {
    String formatted = String(b, HEX);
    if (b < 16) return "0"+formatted;
    return formatted;
}

uint8_t Utility::reverseByte(uint8_t b) {
    b = (b & (uint8_t) 0xF0) >> (uint8_t) 4 | (b & (uint8_t) 0x0F) << 4;
    b = (b & (uint8_t) 0xCC) >> 2 | (b & (uint8_t) 0x33) << 2;
    b = (b & (uint8_t) 0xAA) >> 1 | (b & (uint8_t) 0x55) << 1;
    return b;
}

uint32_t Utility::reverseBits(uint32_t n) {
    uint32_t x = 0;
    for (auto i = 31; n;) {
        x |= (n & 1) << i;
        n >>= 1;
        --i;
    }
    return x;
}
