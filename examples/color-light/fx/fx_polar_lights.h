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
#ifndef HOMEGENIE_MINI_FX_POLAR_LIGHTS_H
#define HOMEGENIE_MINI_FX_POLAR_LIGHTS_H

#include "fx.h"

inline void fx_polar_lights(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f;
    }

    static uint32_t lastUpdate = 0;

    // Execute at ~40 FPS for extremely responsive fluid dynamics
    if (millis() - lastUpdate < 25) return;

    // Calculate Delta-Time (dt) to ensure smooth, jump-free speed changes
    float dt = (millis() - lastUpdate) / 1000.0f;
    lastUpdate = millis();

    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();

    // 1. EXTRACT REAL-TIME FFT AMPLITUDES (Virtual FFT through reverse HSV-to-RGB)
    auto rgb = ::Utility::hsv2rgb(masterH, masterS, masterV);
    float bassAmp = static_cast<float>(rgb.r) / 255.0f;
    float midAmp  = static_cast<float>(rgb.g) / 255.0f;
    float trebAmp = static_cast<float>(rgb.b) / 255.0f;

    // 2. AUDIO-REACTIVE SPEED (Mid frequencies drive the visual undulation speed)
    static float t = 0.0f;
    // Base speed is 1.2x, but strong mids push it up to 4.2x speed!
    float speedMultiplier = 1.2f + (3.0f * midAmp);
    t += dt * speedMultiplier;

    // 3. AUDIO-REACTIVE THICKNESS (Bass hits expand the aurora vertically)
    float auroraThickness = 0.25f + (0.45f * bassAmp);
    float fadeFactor = (cfg.rows > 1) ? (1.0f / (cfg.rows * auroraThickness)) : 0.0f;

    float cy = (cfg.rows - 1) / 2.0f;

    for (uint16_t x = 0; x < cfg.cols; x++) {

        // CPU OPTIMIZATION: Pre-calculate the X-dependent components of the 2D Noise outside the Y loop.
        // This saves hundreds of trigonometric sine/cosine calculations per frame!
        float n1 = sin(static_cast<float>(x) * 0.4f + t);
        float n2_base = static_cast<float>(x) * 0.2f - t * 0.8f;

        // 4. AUDIO-REACTIVE SHIMMER (Treble drives the high-frequency jitter)
        float trebJitter = t * (1.2f + (2.5f * trebAmp));
        float n3_base = static_cast<float>(x) * 0.1f + trebJitter;

        for (uint16_t y = 0; y < cfg.rows; y++) {

            // Reconstruct the 2D Plasma Cloud using the pre-calculated X bases
            float n2 = sin(n2_base + static_cast<float>(y) * 0.3f);
            float n3 = cos(n3_base - static_cast<float>(y) * 0.4f);

            // Noise range scaled to [0.0, 1.0]
            float noise = (n1 + n2 + n3 + 3.0f) / 6.0f;

            float brightness = noise;
            if (cfg.rows > 1) {
                float distY = std::abs(static_cast<float>(y) - cy);
                brightness -= (distY * fadeFactor);
            }

            // Constrain brightness
            if (brightness < 0.0f) brightness = 0.0f;
            if (brightness > 1.0f) brightness = 1.0f;

            // 5. AUDIO-REACTIVE COLOR CORE
            // Bright pixels shift hue (creates the distinct core color, like green edges with yellow center).
            // Treble spikes add an extra flash to the core color, reacting to hi-hats/cymbals!
            float pixelHue = masterS + (brightness * (0.12f + 0.10f * trebAmp));

            if (pixelHue > 1.0f) pixelHue -= 1.0f;
            if (pixelHue < 0.0f) pixelHue += 1.0f;

            // Remap index based on physical layout configuration
            uint16_t idx = getLEDIndex(x, y, cfg);
            if (idx < cfg.count) {
                // Soft 200ms transition ensures a buttery-smooth fade without blurring the FFT reactivity
                animatedColors[idx]->setColor(pixelHue, masterH, brightness * masterV, 200);
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_POLAR_LIGHTS_H
