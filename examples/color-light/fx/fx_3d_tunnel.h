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
#ifndef HOMEGENIE_MINI_FX_3D_TUNNEL_H
#define HOMEGENIE_MINI_FX_3D_TUNNEL_H

#include "fx.h"

inline void fx_3d_tunnel(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // 1. Retrieve all Master Color parameters (Hue, Saturation, Value)
    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();
    float masterV = defaultColor.getValue();

    static uint16_t currentRing = 0;
    static uint32_t lastUpdate = 0;

    // Modulate execution speed based on master brightness (pulses faster on beats)
    uint16_t speed = 40 + (uint16_t)(120.0f * (1.0f - masterV));

    if (millis() - lastUpdate >= speed) {
        lastUpdate = millis();

        // Calculate exact center coordinates
        float cx = (cfg.cols - 1) / 2.0f;
        float cy = (cfg.rows - 1) / 2.0f;
        int maxRings = ceil(max(cx, cy));

        // Determine direction based on master hue threshold
        bool outward = (masterH > 0.5f);

        // Safe signed accumulator to prevent unsigned underflow during inward/outward calculations
        int16_t ringTemp = currentRing;
        if (outward) {
            ringTemp++;
            if (ringTemp > maxRings) ringTemp = 0;
        } else {
            ringTemp--;
            if (ringTemp < 0) ringTemp = maxRings;
        }
        currentRing = (uint16_t)ringTemp;

        // Anchor tunnel color to master Hue, adding a slight shift per ring for 3D depth
        float ringHue = masterH + (currentRing * 0.03f);
        if (ringHue > 1.0f) ringHue -= 1.0f;

        // Dynamically compute the trailing ring index to fade out based on current direction
        uint16_t trailingRing = outward ? (currentRing - 1) : (currentRing + 1);
        if (outward && currentRing == 0) trailingRing = maxRings;
        if (!outward && currentRing == maxRings) trailingRing = 0;

        // Render full frame using standard Row-Major Serpentine mapping
        for (uint16_t x = 0; x < cfg.cols; x++) {
            for (uint16_t y = 0; y < cfg.rows; y++) {

                // Chebyshev distance calculation for concentric squares
                int ring = (int)max(std::abs((float)x - cx), std::abs((float)y - cy));

                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(x, y, cfg);
                if (idx >= cfg.count) continue;

                // 1. Current leading ring (turns on instantly)
                if (ring == currentRing) {
                    animatedColors[idx]->setColor(ringHue, masterS, masterV, 0);
                }
                // 2. Trailing rings (fade to black slowly to create depth)
                // Decay time is scaled with speed (speed * 6) to keep trails consistent when pulsing
                else if (ring == trailingRing) {
                    animatedColors[idx]->setColor(ringHue, masterS, 0.0f, speed * 6);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_3D_TUNNEL_H
