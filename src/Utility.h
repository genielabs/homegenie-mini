/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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


typedef struct ColorRGB {
    int r;
    int g;
    int b;
} ColorRGB; // renamed from RGBColor to ColorRGB to prevent conflicts with LGFX

typedef struct ColorHSV {
    float h;
    float s;
    float v;
} ColorHSV;


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
    static ColorRGB hsv2rgb(float H, float S, float V);
    static ColorHSV rgb2hsv(float r, float g, float b);
    static uint32_t getFreeMem();
    static time_t relativeUtcHoursToLocalTime(double relativeHours, time_t time);
    static void simpleJsonStringEscape(String& s);
    static bool isNumeric(const char* s) {
        return strlen(s) > 0 && strspn(s, "0123456789.") == strlen(s);
    }
    static float roundDecimals(float value, unsigned int n_decimals = 1) {
        float multiplier = std::pow(10.0f, static_cast<float>(n_decimals));
        return std::round(value * multiplier) / multiplier;
    }
private:
    static float max(float a, float b, float c) {
        return ((a > b)? (a > c ? a : c) : (b > c ? b : c));
    }
    static float min(float a, float b, float c) {
        return ((a < b)? (a < c ? a : c) : (b < c ? b : c));
    }
};

#endif //HOMEGENIE_MINI_UTILITY_H
