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

#include "Utility.h"

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
        data[i] = strtol(tmp, nullptr, 16);
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
