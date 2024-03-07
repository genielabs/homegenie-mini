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
 * - 2019-01-28 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_UTILITY_H
#define HOMEGENIE_MINI_UTILITY_H

#include "Config.h"


typedef struct RGBColor {
    int r;
    int g;
    int b;
} RGBColor;


class Utility {
public:
    static void getBytes(const String &rawBytes, uint8_t *data);
    static void getBytes(const String &rawBytes, uint16_t *data);
    static String getByteString(uint8_t *data, uint16_t length);
    static String getByteString(uint16_t *data, uint16_t length);
    static String getByteString(uint64_t *data, uint16_t length);
    static String byteToHex(byte b);
    static String byteToHex(int16_t b);
    static String byteToHex(uint16_t b);
    static String byteToHex(uint32_t b);
    static String byteToHex(uint64_t b);
    static uint32_t reverseBits(uint32_t n);
    static uint8_t reverseByte(uint8_t n);
    static RGBColor hsv2rgb(float H, float S, float V);
};

#endif //HOMEGENIE_MINI_UTILITY_H
