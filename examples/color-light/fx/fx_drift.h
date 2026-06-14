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
#ifndef HOMEGENIE_MINI_FX_DRIFT_H
#define HOMEGENIE_MINI_FX_DRIFT_H

#include "fx.h"

inline void fx_drift(const MatrixConfig& cfg, const LightColor& defaultColor) {
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

    // High quality motion blur (150ms) to merge orbital steps into smooth glowing trails
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 150);
    }

    // Calculate center coordinates
    int16_t colsCenter = (cfg.cols >> 1);
    int16_t rowsCenter = (cfg.rows >> 1);

    // Calculate maximum radius (half of the largest dimension)
    float maxDim = static_cast<float>(max(cfg.cols, cfg.rows)) / 2.0f;

    // Time-based slow rotation timer
    uint32_t t = millis() / 16;
    uint32_t t_20 = t / 20;

    // Subsampled polar loop (extremely lightweight: exactly 32 iterations for 16x16 matrix)
    for (float i = 1.0f; i < maxDim; i += 0.25f) {
        // Convert degrees-based time rotation to radians
        float angle = (t * (maxDim - i)) * (3.14159265f / 180.0f);

        int16_t mySin = (int16_t)round(sin(angle) * i);
        int16_t myCos = (int16_t)round(cos(angle) * i);

        // Dynamically shift rainbow hue based on distance (i) and rotation time (t_20)
        float hueOffset = static_cast<float>((int16_t) (i * 20.0f) + t_20) / 255.0f;
        while (hueOffset >= 1.0f) hueOffset -= 1.0f;
        float pixelHue = masterH + hueOffset;
        if (pixelHue > 1.0f) pixelHue -= 1.0f;

        // 1. Primary spiral point (collapses into a 1D bounce scanner if rows == 1)
        int16_t px1 = colsCenter + mySin;
        int16_t py1 = (cfg.rows > 1) ? (rowsCenter + myCos) : 0;
        if (px1 >= 0 && px1 < cfg.cols && py1 >= 0 && py1 < cfg.rows) {
            // Remap index based on physical layout configuration
            uint16_t idx1 = getLEDIndex(px1, py1, cfg);
            if (idx1 < cfg.count) {
                animatedColors[idx1]->setColor(pixelHue, masterS, masterV, 0);
            }
        }

        // 2. Twin symmetric point (renders a gorgeous dual-spiral mandala in 2D)
        if (cfg.rows > 1) {
            int16_t px2 = colsCenter + myCos;
            int16_t py2 = rowsCenter + mySin;
            if (px2 >= 0 && px2 < cfg.cols && py2 >= 0 && py2 < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx2 = getLEDIndex(px2, py2, cfg);
                if (idx2 < cfg.count) {
                    animatedColors[idx2]->setColor(pixelHue, masterS, masterV, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_DRIFT_H
