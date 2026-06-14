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
#ifndef HOMEGENIE_MINI_FX_FIRE_H
#define HOMEGENIE_MINI_FX_FIRE_H

#include "fx.h"

// Helper function for saturating 8-bit addition
inline uint8_t qadd8(uint8_t a, uint8_t b) {
    uint16_t sum = (uint16_t)a + b;
    return (sum > 255) ? 255 : (uint8_t)sum;
}

inline void fx_fire(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Static heat map buffer supporting up to 1024 LEDs (e.g. 32x32 matrix)
    const uint16_t MAX_LEDS = 1024;
    static uint8_t heat[MAX_LEDS] = {0};
    static uint32_t lastUpdate = 0;

    // Run fire simulation at a smooth 30ms rate (~33 FPS)
    if (millis() - lastUpdate < 30) {
        return;
    }
    lastUpdate = millis();

    // 1. Retrieve Master Colors and Dimmer values
    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();
    float masterV = defaultColor.getValue();
    //if (masterV <= 0.01f) masterV = 1.0f; // Prevent invisible rendering

    // 2. Modulate fire physics based on Master Dimmer (value)
    // Low masterV -> high cooling (short fire). High masterV -> low cooling (tall fire)
    uint16_t cooling_mod = 25 + (uint16_t)(80.0f * (1.0f - masterV));

    // Low masterV -> fewer sparks. High masterV -> dense sparks
    uint8_t sparking_prob = 50 + (uint8_t)(150.0f * masterV);

    // 1D Fire2012 implementation
    if (cfg.rows == 1) {
        for (uint16_t i = 0; i < cfg.count; i++) {
            if (i >= MAX_LEDS) break;
            uint8_t cooldown = random(0, 15);
            heat[i] = (cooldown > heat[i]) ? 0 : (heat[i] - cooldown);
        }
        for (uint16_t i = cfg.count - 1; i >= 2; i--) {
            if (i >= MAX_LEDS) continue;
            heat[i] = ((uint16_t)heat[i-1] + heat[i-2]) / 2;
        }
        if (random(0, 255) < sparking_prob) {
            heat[0] = qadd8(heat[0], random(160, 255));
            if (cfg.count > 1) {
                heat[1] = qadd8(heat[1], random(100, 200));
            }
        }
    }
    // 2D Fire2012 implementation
    else {
        // Cool down every cell dynamically modulated by master brightness
        for (uint16_t i = 0; i < cfg.count; i++) {
            if (i >= MAX_LEDS) break;
            uint8_t cooldown = random(0, ((cooling_mod * 10) / cfg.rows) + 2);
            heat[i] = (cooldown > heat[i]) ? 0 : (heat[i] - cooldown);
        }

        // Drift heat physically from bottom to top (Row r+1 to Row r)
        for (uint16_t r = 0; r < cfg.rows - 1; r++) {
            for (uint16_t c = 0; c < cfg.cols; c++) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(c, r, cfg);
                if (idx >= cfg.count || idx >= MAX_LEDS) continue;

                uint16_t r_under = r + 1;
                uint16_t idx_under = getLEDIndex(c, r_under, cfg);

                uint16_t c_left = (c > 0) ? c - 1 : 0;
                uint16_t idx_under_left = getLEDIndex(c_left, r_under, cfg);

                uint16_t c_right = (c < cfg.cols - 1) ? c + 1 : cfg.cols - 1;
                uint16_t idx_under_right = getLEDIndex(c_right, r_under, cfg);

                if (idx_under < cfg.count && idx_under_left < cfg.count && idx_under_right < cfg.count) {
                    heat[idx] = ((uint16_t)heat[idx_under] + heat[idx_under_left] + heat[idx_under_right]) / 3;
                }
            }
        }

        // Spark new heat randomly at the bottom row (y = rows - 1)
        if (random(0, 255) < sparking_prob) {
            uint16_t c = random(0, cfg.cols);
            uint16_t r_bottom = cfg.rows - 1;
            // Remap index based on physical layout configuration
            uint16_t idx = getLEDIndex(c, r_bottom, cfg);
            if (idx < cfg.count && idx < MAX_LEDS) {
                heat[idx] = qadd8(heat[idx], random(160, 255));
            }
        }
    }

    // 3. RENDER HEAT MAP VALUES (0-255) TO REALISTIC FIRE HUE-SHIFTING COLORS
    for (uint16_t i = 0; i < cfg.count; i++) {
        if (i >= MAX_LEDS) break;

        uint8_t hVal = heat[i];
        float h = masterH;
        float s = masterS;
        float v = 0.0f;

        if (hVal > 170) {
            // Hot core: Color shifts slightly and fades saturation to white-hot
            h = masterH + 0.12f;
            s = ((float)(255 - hVal) / 85.0f) * masterS; // Desaturate base color for hot core glow
            v = 1.0f;
        } else if (hVal > 85) {
            // Medium heat: Color shifts dynamically from base hue
            float factor = (float)(hVal - 85) / 85.0f;
            h = masterH + (0.12f * factor);
            s = masterS;
            v = 1.0f;
        } else if (hVal > 15) {
            // Cool: Base hue fading out to black
            h = masterH;
            s = masterS;
            v = (float)hVal / 85.0f;
        }

        // Keep calculated hue within 0.0 - 1.0 boundaries
        if (h > 1.0f) h -= 1.0f;

        // Set final color mapped through master dimmer
        animatedColors[i]->setColor(h, s, v * masterV, 0);
    }
}

#endif //HOMEGENIE_MINI_FX_FIRE_H
