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
#ifndef HOMEGENIE_MINI_FX_PARTICLE_GEQ_H
#define HOMEGENIE_MINI_FX_PARTICLE_GEQ_H

#include "fx.h"

struct GeqParticle {
    float x, y;
    float vx, vy;
    float hue;
    float age;
    bool active;
};

inline void fx_particle_geq(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Retrieve the master HSV parameters sent by the server
    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();
    float masterV = defaultColor.getValue();

    constexpr uint8_t MAX_PARTICLES = 48;
    static GeqParticle particles[MAX_PARTICLES];
    static uint32_t lastUpdate = 0;
    static bool initialized = false;

    // Initialize the particle pool on first run
    if (!initialized) {
        for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
        initialized = true;
    }

    // Limit graphics refresh to 25ms (~40 FPS) for smooth motion
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    // Creates a motion blur effect by fading preceding pixels toward zero
    // but maintaining the user-selected hue and saturation.
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 150);
    }

    // Convert the master HSV to RGB to extract the actual audio amplitudes (R=Bass, G=Mid, B=Treble)
    // NOTE: Make sure masterH, masterS, and masterV are scaled correctly before hsv2rgb if required by the library.
    auto rgb = ::Utility::hsv2rgb(masterH, masterS, masterV);
    float bassAmp = static_cast<float>(rgb.r) / 255.0f;
    float midAmp  = static_cast<float>(rgb.g) / 255.0f;
    float trebAmp = static_cast<float>(rgb.b) / 255.0f;

    // Map the 3 amplitudes onto 16 virtual bands by applying organic random jitter
    float fftResult[16];
    for (uint8_t bin = 0; bin < 16; bin++) {
        if (bin < 5) {
            // Bass bands (0..4) driven by the Red channel
            fftResult[bin] = bassAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        else if (bin < 11) {
            // Midrange bands (5..10) driven by the Green channel
            fftResult[bin] = midAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        else {
            // High bands (11..15) driven by the Blue channel
            fftResult[bin] = trebAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        if (fftResult[bin] > 1.0f) fftResult[bin] = 1.0f;
    }

    // 2. DRAW EQUALIZER COLUMNS (VU-METER) AND GENERATE SPARKS ON THE PEAK
    for (uint16_t c = 0; c < cfg.cols; c++) {
        uint8_t bin = (c * 16) / cfg.cols;

        // The height of the column depends on the spectral value and the masterV volume
        uint16_t barHeight = (uint16_t)(fftResult[bin] * masterV * static_cast<float>(cfg.rows));
        if (barHeight > cfg.rows) barHeight = cfg.rows;

        // Classic color gradient for spectrum (Green -> Yellow -> Red)
        float barHue = (static_cast<float>(bin) / 16.0f) * 0.85f;

        // Draw the solid column
        for (uint16_t r = 0; r < barHeight; r++) {
            uint16_t py = cfg.rows - 1 - r;
            // Remap index based on physical layout configuration
            uint16_t idx = getLEDIndex(c, py, cfg);
            if (idx < cfg.count) {
                animatedColors[idx]->setColor(barHue, 1.0f, 1.0f, 0);
            }
        }

        // Generate a particle/spark at the column peak if the intensity is high
        if (cfg.rows > 1 && fftResult[bin] > 0.4f && random(0, 100) < 30) {
            for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
                if (!particles[p].active) {
                    particles[p].active = true;
                    particles[p].x = c;
                    particles[p].y = static_cast<float>(cfg.rows - 1 - barHeight);
                    particles[p].vx = static_cast<float>(random(-10, 10)) / 40.0f; // Splash laterale
                    particles[p].vy = -(fftResult[bin] * 0.3f) - 0.1f; // Spinta verso l'alto proporzionale all'ampiezza
                    particles[p].hue = barHue;
                    particles[p].age = 1.0f;
                    break; // Generate at most one particle per frame column
                }
            }
        }
    }

    // 3. UPDATE AND DRAW ACTIVE PARTICLES WITH GRAVITY
    float gravity = (cfg.rows > 1) ? 0.05f : 0.0f;
    for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
        if (particles[p].active) {
            // Apply the velocity vectors and the gravity deceleration
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;
            particles[p].vy += gravity;
            particles[p].age -= 0.04f; // Duration of about 25 frames (~600ms)

            // Round the float position before doing out-of-bounds checks and rendering
            int px = static_cast<int>(round(particles[p].x));
            int py = static_cast<int>(round(particles[p].y));

            // STABILITY BUG FIX:
            // If the particle is exhausted or its rounded px/py coordinate goes outside the physical bounds of the matrix,
            // we immediately deactivate it, avoiding LED index calculation errors.
            if (particles[p].age <= 0.0f || px < 0 || px >= cfg.cols || py < 0 || py >= cfg.rows) {
                particles[p].active = false;
                continue;
            }

            // Calculate particle pixel index and render
            uint16_t idx = getLEDIndex(px, py, cfg);
            if (idx < cfg.count) {
                // The particle gradually fades in brightness (Value) based on its age
                animatedColors[idx]->setColor(particles[p].hue, 1.0f, particles[p].age, 0);
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_PARTICLE_GEQ_H
