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
#ifndef HOMEGENIE_MINI_FX_MATRIX_RAIN_H
#define HOMEGENIE_MINI_FX_MATRIX_RAIN_H

#include "fx.h"

inline void fx_matrix_rain(const MatrixConfig& cfg, const LightColor& defaultColor) {
    // Prevent division-by-zero or unconfigured matrix crashes
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Safety limit for static memory allocation (supports up to 128-column wide matrices)
    const uint16_t MAX_COLS = 128;

    static int dropY[MAX_COLS];
    static uint16_t speed[MAX_COLS];
    static uint32_t lastUpdate[MAX_COLS] = {0};
    static uint16_t lastInitializedCols = 0;

    // Initialize or re-initialize if columns count changes
    if (cfg.cols != lastInitializedCols) {
        uint16_t initCols = (cfg.cols > MAX_COLS) ? MAX_COLS : cfg.cols;
        for (uint16_t c = 0; c < initCols; c++) {
            dropY[c] = random(-cfg.rows, 0);  // Shift start position above the screen
            speed[c] = random(50, 200);   // Fall speed in milliseconds per step
            lastUpdate[c] = millis();
        }
        lastInitializedCols = cfg.cols;
    }

    // Fallback to minimal brightness if target is off to prevent invisible rain
    float defV = defaultColor.getValue();

    // Calculate dynamic tail length proportional to the matrix height (half of the rows, min of 2)
    uint16_t tailMultiplier = cfg.rows >> 1; // equivalent to rows / 2
    if (tailMultiplier < 2) {
        tailMultiplier = 2;
    }

    for (uint16_t c = 0; c < cfg.cols; c++) {
        if (c >= MAX_COLS) break;

        if (millis() - lastUpdate[c] >= speed[c]) {
            lastUpdate[c] = millis();

            int r_head = dropY[c]; // Current head position before advancing

            // 1. Older tail: start fading from the body's brightness down to 0.0f
            int r_older = r_head - 1;
            if (r_older >= 0 && r_older < cfg.rows) {
                uint16_t idx_older = getLEDIndex(c, r_older, cfg);
                if (idx_older < cfg.count) {
                    // Set proportional transition time to fade down smoothly to black
                    animatedColors[idx_older]->setColor(
                        defaultColor.getTargetHue(),
                        defaultColor.getTargetSaturation(),
                        0.0f,
                        speed[c] * tailMultiplier
                    );
                }
            }

            // 2. Immediate body: drop the old head's brightness quickly to create a steep gradient
            int r_old = r_head;
            if (r_old >= 0 && r_old < cfg.rows) {
                uint16_t idx_old = getLEDIndex(c, r_old, cfg);
                if (idx_old < cfg.count) {
                    // Drop instantly to 40% of max brightness (fast 50ms transition)
                    animatedColors[idx_old]->setColor(
                        defaultColor.getTargetHue(),
                        defaultColor.getTargetSaturation(),
                        defV * 0.4f, // Adjust this multiplier (e.g. 0.3f to 0.5f) to tune the contrast
                        50
                    );
                }
            }

            // 3. Advance drop position
            dropY[c]++;

            // Respawn drop above screen if it has fully drained off the bottom.
            // We let it go up to rows + 1 so the last row (rows - 1) has a chance to fade to 0.0f.
            if (dropY[c] >= cfg.rows + 1) {
                dropY[c] = random(-4, 0);
                speed[c] = random(50, 200);
            }

            // 4. Render new head position
            int r_new = dropY[c];
            if (r_new >= 0 && r_new < cfg.rows) {
                uint16_t idx_new = getLEDIndex(c, r_new, cfg);
                if (idx_new < cfg.count) {
                    // Instant onset for the leading drop head (transitionMs = 50)
                    animatedColors[idx_new]->setColor(
                        defaultColor.getTargetHue(),
                        defaultColor.getTargetSaturation(),
                        defV,
                        50
                    );
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_MATRIX_RAIN_H
