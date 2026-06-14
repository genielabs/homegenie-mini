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
#ifndef HOMEGENIE_MINI_FX_WAVERLY_H
#define HOMEGENIE_MINI_FX_WAVERLY_H

#include "fx.h"

inline void fx_waverly(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Retrieve master value to scale the wave height instead of overall pixel brightness
    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f;
    }

    static uint32_t lastUpdate = 0;

    // Limit updates to 25ms (~40 FPS) for smooth wave rendering
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    // Standard HSV mapping (unswapped) to perfectly respect the user's chosen color
    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();

    // Slowly advancing timebase
    float t = millis() / 400.0f;

    // Apply a gentle motion-blur fade-out to black
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 120);
    }

    for (uint16_t i = 0; i < cfg.cols; i++) {
        // Generate organic 1D wave value for this column (approximate perlin8)
        float wave = sin((float)i * 0.35f + t) + cos((float)i * 0.7f - t * 0.6f);
        float normNoise = (wave + 2.0f) / 4.0f; // Scale to [0.0, 1.0]

        // Calculate wave height scaled dynamically by the master value (volume)
        uint16_t thisMax = (uint16_t)(normNoise * masterV * (float)cfg.rows);
        if (thisMax > cfg.rows) thisMax = cfg.rows;

        for (uint16_t j = 0; j < thisMax; j++) {
            // Symmetrical color gradient from the base to the tip of the wave
            float factor = (float)j / (float)thisMax;
            float pixelHue = masterH + (factor * 0.15f); // Shifts from base hue (e.g. Green to Cyan)
            if (pixelHue > 1.0f) pixelHue -= 1.0f;

            // 1. Bottom-left wave
            // Remap index based on physical layout configuration
            uint16_t idx1 = getLEDIndex(i, j, cfg);
            if (idx1 < cfg.count) {
                animatedColors[idx1]->setColor(pixelHue, masterS, 1.0f, 0); // Crisp full-brightness pixels
            }

            // 2. Top-right mirrored wave (Renders a gorgeous double-interleaved waveform in 2D)
            if (cfg.rows > 1) {
                uint16_t rx = (cfg.cols - 1) - i;
                uint16_t ry = (cfg.rows - 1) - j;
                // Remap index based on physical layout configuration
                uint16_t idx2 = getLEDIndex(rx, ry, cfg);
                if (idx2 < cfg.count) {
                    animatedColors[idx2]->setColor(pixelHue, masterS, 1.0f, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_WAVERLY_H
