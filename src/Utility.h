//
// Created by gene on 28/01/19.
//

#ifndef HOMEGENIE_MINI_UTILITY_H
#define HOMEGENIE_MINI_UTILITY_H

#include <Arduino.h>

class Utility {
public:
    static void getBytes(const String &rawBytes, uint8_t *data);
    static String byteToHex(byte b);
    static uint32_t reverseBits(uint32_t n);
    static uint8_t reverseByte(uint8_t n);
};


#endif //HOMEGENIE_MINI_UTILITY_H
