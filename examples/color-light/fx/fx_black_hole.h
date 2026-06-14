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
#ifndef HOMEGENIE_MINI_FX_BLACK_HOLE_H
#define HOMEGENIE_MINI_FX_BLACK_HOLE_H

#include "fx.h"

// High-precision beatsin generator for 8-bit phase calculations
inline uint16_t beatsin8_local(float bpm, uint16_t low, uint16_t high, float phase) {
    float phase_rad = phase * (2.0f * 3.14159265f / 255.0f);
    float omega_t = ((2.0f * 3.14159265f * bpm * millis()) / 60000.0f) + phase_rad;
    float sin_val = (sin(omega_t) + 1.0f) / 2.0f;
    return low + (uint16_t)(sin_val * (high - low));
}

inline void fx_black_hole(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Retrieve master brightness and enforce safe shut down if master is 0
    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f;
    }

    static uint32_t lastUpdate = 0;

    // Limit updates to 25ms (~40 FPS) for smooth visual motion
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();

    // Apply a gentle motion blur using swapped S and H as background
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterS, masterH, 0.0f, 160);
    }

    // Modulate orbit speeds based on master value (beats make orbits faster!)
    float bpmOuterX = 6.0f + (12.0f * masterV);
    float bpmOuterY = 8.0f + (15.0f * masterV);
    float bpmInnerX = 10.0f + (18.0f * masterV);
    float bpmInnerY = 12.0f + (21.0f * masterV);

    uint32_t t = millis() / 128; // Slowly advancing phase base

    // 1. RENDER OUTER STARS (8 of them)
    for (uint8_t i = 0; i < 8; i++) {
        float phaseX = ((i % 2) ? 128.0f : 0.0f) + static_cast<float>(t * i);
        float phaseY = ((i % 2) ? 192.0f : 64.0f) + static_cast<float>(t * i);

        uint16_t x = beatsin8_local(bpmOuterX, 0, cfg.cols - 1, phaseX);
        uint16_t y = (cfg.rows > 1) ? beatsin8_local(bpmOuterY, 0, cfg.rows - 1, phaseY) : 0;

        // Map color using the swapped HSV logic (masterS as base hue)
        float hueOffset = static_cast<float>(i * 32) / 255.0f;
        float pixelHue = masterS + hueOffset;
        if (pixelHue > 1.0f) pixelHue -= 1.0f;

        // Remap index based on physical layout configuration
        uint16_t idx = getLEDIndex(x, y, cfg);
        if (idx < cfg.count) {
            animatedColors[idx]->setColor(pixelHue, masterH, masterV, 0);
        }
    }

    // 2. RENDER INNER STARS (4 of them)
    for (uint8_t i = 0; i < 4; i++) {
        float phaseX = ((i % 2) ? 128.0f : 0.0f) + static_cast<float>(t * i);
        float phaseY = ((i % 2) ? 192.0f : 64.0f) + static_cast<float>(t * i);

        // Inner stars orbit in a tighter bounding box (rows/4 to 3*rows/4)
        uint16_t x = beatsin8_local(bpmInnerX, cfg.cols / 4, cfg.cols - 1 - cfg.cols / 4, phaseX);
        uint16_t y = (cfg.rows > 1) ? beatsin8_local(bpmInnerY, cfg.rows / 4, cfg.rows - 1 - cfg.rows / 4, phaseY) : 0;

        float hueOffset = static_cast<float>(255 - i * 64) / 255.0f;
        float pixelHue = masterS + hueOffset;
        if (pixelHue > 1.0f) pixelHue -= 1.0f;

        // Remap index based on physical layout configuration
        uint16_t idx = getLEDIndex(x, y, cfg);
        if (idx < cfg.count) {
            animatedColors[idx]->setColor(pixelHue, masterH, masterV, 0);
        }
    }

    // 3. RENDER CENTRAL WHITE DOT (The event horizon)
    uint16_t cx = cfg.cols / 2;
    uint16_t cy = (cfg.rows > 1) ? (cfg.rows / 2) : 0;

    // Remap index based on physical layout configuration
    uint16_t centerIdx = getLEDIndex(cx, cy, cfg);
    if (centerIdx < cfg.count) {
        // Pure white dot matching master brightness (LEVEL)
        animatedColors[centerIdx]->setColor(0.0f, 0.0f, masterV, 0);
    }
}

#endif //HOMEGENIE_MINI_FX_BLACK_HOLE_H
