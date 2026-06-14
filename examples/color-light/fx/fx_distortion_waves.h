/*
* HomeGenie-Mini (c) 2018-2026 G-Labs
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
 */
#ifndef HOMEGENIE_MINI_FX_DISTORTION_WAVES_H
#define HOMEGENIE_MINI_FX_DISTORTION_WAVES_H

#include "fx.h"

// Helper structure for HSV color space
struct HSVColor {
    float h, s, v;
};

// Fast O(1) 8-bit Cosine lookup table emulator
inline uint8_t cos8_local(uint8_t val) {
    static uint8_t table[256];
    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < 256; i++) {
            table[i] = (uint8_t)((cos((float)i * 2.0f * 3.14159265f / 255.0f) + 1.0f) * 127.5f);
        }
        initialized = true;
    }
    return table[val];
}

// Emulates WLED's beatsin16 wave generator
inline uint16_t beatsin16_local(float bpm, uint16_t low, uint16_t high) {
    float omega_t = (2.0f * 3.14159265f * bpm * millis()) / 60000.0f;
    float sin_val = (sin(omega_t) + 1.0f) / 2.0f;
    return low + (uint16_t)(sin_val * (high - low));
}

// Fast RGB to HSV conversion helper
inline HSVColor rgb2hsv_local(uint8_t r, uint8_t g, uint8_t b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float maxVal = max(rf, max(gf, bf));
    float minVal = min(rf, min(gf, bf));
    float delta = maxVal - minVal;

    HSVColor hsv;
    hsv.v = maxVal;

    if (maxVal > 0.0f) {
        hsv.s = delta / maxVal;
    } else {
        hsv.s = 0.0f;
        hsv.h = 0.0f;
        return hsv;
    }

    if (delta > 0.0f) {
        if (maxVal == rf) {
            hsv.h = (gf - bf) / delta + (gf < bf ? 6.0f : 0.0f);
        } else if (maxVal == gf) {
            hsv.h = (bf - rf) / delta + 2.0f;
        } else {
            hsv.h = (rf - gf) / delta + 4.0f;
        }
        hsv.h /= 6.0f;
    } else {
        hsv.h = 0.0f;
    }

    return hsv;
}

inline void fx_distortion_waves(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    static uint32_t lastUpdate = 0;
    // Limit execution to ~33 FPS to keep CPU load low
    if (millis() - lastUpdate < 30) {
        return;
    }
    lastUpdate = millis();

    // Default speed and scale parameters optimized for 16x16 grid
    uint8_t speed = 4;  // Speed divisor
    uint8_t scale = 12; // Wave scale/zoom factor

    unsigned a  = millis() / 32;
    unsigned a2 = a / 2;
    unsigned a3 = a / 3;
    unsigned colsScaled = cfg.cols * scale;
    unsigned rowsScaled = cfg.rows * scale;

    // Displace wave centers using beatsin16 emulators
    unsigned cx  = beatsin16_local(10.0f - speed, 0, colsScaled);
    unsigned cy  = beatsin16_local(12.0f - speed, 0, rowsScaled);
    unsigned cx1 = beatsin16_local(13.0f - speed, 0, colsScaled);
    unsigned cy1 = beatsin16_local(15.0f - speed, 0, rowsScaled);
    unsigned cx2 = beatsin16_local(17.0f - speed, 0, colsScaled);
    unsigned cy2 = beatsin16_local(14.0f - speed, 0, rowsScaled);

    float masterV = defaultColor.getValue();
    //if (masterV <= 0.01f) masterV = 1.0f;

    unsigned xoffs = 0;
    for (uint16_t x = 0; x < cfg.cols; x++) {
        xoffs += scale;
        unsigned yoffs = 0;

        for (uint16_t y = 0; y < cfg.rows; y++) {
            yoffs += scale;

            uint8_t rdistort, gdistort, bdistort;

            // Generate distortion values for R, G and B channels using O(1) cos8 emulators
            rdistort = cos8_local((cos8_local(((x << 3) + a) & 255) + cos8_local(((y << 3) - a2) & 255) + a3) & 255) >> 1;
            gdistort = cos8_local((cos8_local(((x << 3) - a2) & 255) + cos8_local(((y << 3) + a3) & 255) + a + 32) & 255) >> 1;
            bdistort = cos8_local((cos8_local(((x << 3) + a3) & 255) + cos8_local(((y << 3) - a) & 255) + a2 + 64) & 255) >> 1;

            uint16_t valR = rdistort + ((a - (((xoffs - cx) * (xoffs - cx) + (yoffs - cy) * (yoffs - cy)) >> 7)) << 1);
            uint16_t valG = gdistort + ((a2 - (((xoffs - cx1) * (xoffs - cx1) + (yoffs - cy1) * (yoffs - cy1)) >> 7)) << 1);
            uint16_t valB = bdistort + ((a3 - (((xoffs - cx2) * (xoffs - cx2) + (yoffs - cy2) * (yoffs - cy2)) >> 7)) << 1);

            uint8_t valueR = cos8_local((uint8_t)valR);
            uint8_t valueG = cos8_local((uint8_t)valG);
            uint8_t valueB = cos8_local((uint8_t)valB);

            // Convert calculated RGB to HSV space for HomeGenie compliance
            HSVColor hsv = rgb2hsv_local(valueR, valueG, valueB);
            hsv.v = hsv.v * masterV; // Scale with master dimmer

            // Remap index based on physical layout configuration
            uint16_t idx = getLEDIndex(x, y, cfg);
            if (idx >= cfg.count) continue;

            animatedColors[idx]->setColor(hsv.h, hsv.s, hsv.v, 0);
        }
    }
}

#endif //HOMEGENIE_MINI_FX_DISTORTION_WAVES_H
