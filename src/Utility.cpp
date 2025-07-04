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

#include "Utility.h"

/// Convert byte to hex string taking care of leading-zero
/// \param b
void Utility::getBytes(const String &rawBytes, uint8_t *data) {
    uint16_t dataSize = rawBytes.length() / 2;
    for (int h = 0; h < dataSize; h++) {
        String hex = rawBytes.substring(h*2, (h*2) + 2);
        auto v = (uint8_t)strtol(hex.c_str(), nullptr, 16);
        data[h] = v;
    }
}
void Utility::getBytes(const String &rawBytes, uint16_t *data) {
    uint16_t dataSize = rawBytes.length() / 4;
    for (int h = 0; h < dataSize; h++) {
        String hex = rawBytes.substring(h*4, (h*4) + 4);
        auto v = (uint16_t)strtol(hex.c_str(), nullptr, 16);
        data[h] = v;
    }
}
String Utility::byteToHex(byte b) {
    String formatted = String(b, HEX);
    if (b < 16) return "0"+formatted;
    return formatted;
}
String Utility::byteToHex(int16_t b) {
    String formatted = String(b, HEX);
    while (formatted.length() < 4) formatted = String("0") + formatted;
    return formatted;
}
String Utility::byteToHex(uint16_t b) {
    String formatted = String(b, HEX);
    while (formatted.length() < 4) formatted = String("0") + formatted;
    return formatted;
}
String Utility::byteToHex(uint32_t b) {
    String formatted = String(b, HEX);
    while (formatted.length() < 8) formatted = String("0") + formatted;
    return formatted;
}
String Utility::byteToHex(uint64_t b) {
#ifndef ESP8266
    String formatted = String(b, HEX);
#else
    String formatted = String((uint32_t)b, HEX);
#endif
    while (formatted.length() < 16) formatted = String("0") + formatted;
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

String Utility::getByteString(uint16_t *data, uint16_t length) {
    String stringData;
    for(int i = 0; i < length ; i++)
    {
        stringData += byteToHex(data[i]);
    }
    return stringData;
}

String Utility::getByteString(uint8_t *data, uint16_t length) {
    String stringData;
    for(int i = 0; i < length ; i++)
    {
        stringData += byteToHex(data[i]);
    }
    return stringData;
}

String Utility::getByteString(uint64_t *data, uint16_t length) {
    String stringData;
    for(int i = 0; i < length ; i++)
    {
        stringData += byteToHex(data[i]);
    }
    return stringData;
}

ColorHSV Utility::rgb2hsv(float r, float g, float b) {
    // R, G, B values are divided by 255
    // to change the range from 0..255 to 0..1:
    float h, s, v;
    r /= 255.0;
    g /= 255.0;
    b /= 255.0;
    float cmax = max(r, g, b); // maximum of r, g, b
    float cmin = min(r, g, b); // minimum of r, g, b
    float diff = cmax-cmin; // diff of cmax and cmin.
    float degmax = 360.0f;
    if (cmax == cmin)
        h = 0;
    else if (cmax == r)
        h = fmod((60 * ((g - b) / diff) + 360), degmax);
    else if (cmax == g)
        h = fmod((60 * ((b - r) / diff) + 120), degmax);
    else if (cmax == b)
        h = fmod((60 * ((r - g) / diff) + 240), degmax);
    // if cmax equal zero
    if (cmax == 0)
        s = 0;
    else
        s = (diff / cmax) * 100;
    // compute v
    v = cmax * 100;
    return ColorHSV{h / degmax, s / 100.0f, v / 100.0f};
}

ColorRGB Utility::hsv2rgb(float h, float s, float v) {
    float r, g, b;

    int i = floor(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
        case 0: r = v, g = t, b = p; break;
        case 1: r = q, g = v, b = p; break;
        case 2: r = p, g = v, b = t; break;
        case 3: r = p, g = q, b = v; break;
        case 4: r = t, g = p, b = v; break;
        case 5: r = v, g = p, b = q; break;
    }

    ColorRGB color;
    color.r = r * 255;
    color.g = g * 255;
    color.b = b * 255;

    return color;
}

uint32_t Utility::getFreeMem() {
#ifdef ESP8266
    uint32_t freeMem = system_get_free_heap_size();
#else
    uint32_t freeMem = esp_get_free_heap_size();
#endif
    return freeMem;
}

time_t Utility::relativeUtcHoursToLocalTime(double relativeHours, time_t dt)  {
    relativeHours = relativeHours + ((double)Config::zone.offset / 60.0);
    int m = int(round(relativeHours * 60));
    int hr = (m / 60) % 24;
    int mn = m % 60;
    struct tm* tm_struct = localtime(&dt);
    tm_struct->tm_hour = hr;
    tm_struct->tm_min = mn;
    tm_struct->tm_sec = 0;
    // TODO: should also set DST??
    return mktime(tm_struct);
}

void Utility::simpleJsonStringEscape(String& s) {
    s.replace("\\", "\\\\");
    s.replace("\"", "\\\"");
    s.replace("\b", "\\b");
    s.replace("\f", "\\f");
    s.replace("\n", "\\n");
    s.replace("\r", "\\r");
    s.replace("\t", "\\t");

    String result = "";
    if (s.length() > 0) {
        result.reserve(s.length() + (s.length() / 2));
    }

    for (int i = 0; i < s.length(); i++) {
        char ch = s.charAt(i);

        if (ch >= '\x00' && ch <= '\x1f') {
            char escaped[7];
            sprintf(escaped, "\\u%04x", (unsigned int)ch);
            result += escaped;
        } else {
            result += ch;
        }
    }
    s = result;
}

bool Utility::isNumeric(const char *s) {
    if (s == nullptr || *s == '\0') {
        return false;
    }
    int i = 0;
    if (s[0] == '+' || s[0] == '-') {
        i = 1;
    }
    if (s[i] == '\0') {
        return false;
    }
    bool hasDecimalPoint = false;
    for (; s[i] != '\0'; i++) {
        if (s[i] == '.') {
            if (hasDecimalPoint) {
                return false;
            }
            hasDecimalPoint = true;
        } else if (!isdigit(s[i])) {
            return false;
        }
    }
    if ((s[0] == '-' || s[0] == '+') && s[1] == '.' && s[2] == '\0') {
        return false;
    }
    if (s[0] == '.' && s[1] == '\0') {
        return false;
    }
    return true;
}

float Utility::roundDecimals(float value, unsigned int n_decimals) {
    float multiplier = std::pow(10.0f, static_cast<float>(n_decimals));
    return std::round(value * multiplier) / multiplier;
}

Utility::UrlParts Utility::parseURL(const String &url) {
    UrlParts parts;
    parts.url = url;
    int protocolEndPos = url.indexOf("://");
    if (protocolEndPos != -1) {
        parts.protocol = url.substring(0, protocolEndPos);
    }
    // credentials lookup
    int atPos = (protocolEndPos != -1) ? url.indexOf('@', protocolEndPos + 3) : url.indexOf('@');
    if (atPos == -1) {
        return parts;
    }
    String protocolPart = (protocolEndPos != -1) ? url.substring(0, protocolEndPos + 3) : "";
    String credentials = url.substring(protocolPart.length(), atPos);
    String hostAndPath = url.substring(atPos + 1);
    // build clean URL
    parts.url = protocolPart + hostAndPath;
    // parse credentials
    int colonPos = credentials.indexOf(':');
    if (colonPos != -1) {
        parts.username = urlDecode(credentials.substring(0, colonPos));
        parts.password = urlDecode(credentials.substring(colonPos + 1));
    } else {
        parts.username = urlDecode(credentials);
    }
    return parts;
}

String Utility::urlDecode(const String &str) {
    String encodedString = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++){
        c=str.charAt(i);
        if (c == '+') {
            encodedString += ' ';
        } else if (c == '%') {
            i++;
            code0=str.charAt(i);
            i++;
            code1=str.charAt(i);
            c = (hexToInt(code0) << 4) | hexToInt(code1);
            encodedString += c;
        } else {
            encodedString += c;
        }
    }
    return encodedString;
}
