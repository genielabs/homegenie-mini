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
#ifndef HOMEGENIE_MINI_FX_PARTICLE_CENTER_GEQ_H
#define HOMEGENIE_MINI_FX_PARTICLE_CENTER_GEQ_H

#include "fx.h"

struct NovaParticle {
    float x, y;
    float vx, vy;
    float hue;
    float age;
    bool active;
};

inline void fx_particle_center_geq(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Retrieve master brightness and enforce safe shut down if master is 0
    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f;
    }

    const uint8_t MAX_PARTICLES = 48;
    static NovaParticle particles[MAX_PARTICLES];
    static float nozzleAngle = 0.0f;
    static uint32_t lastUpdate = 0;
    static bool initialized = false;

    // Initialize particle pool on first execution
    if (!initialized) {
        for (uint8_t i = 0; i < MAX_PARTICLES; i++) {
            particles[i].active = false;
        }
        initialized = true;
    }

    // Limit physics updates to 25ms (~40 FPS) for smooth circular motion
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();

    // High motion blur (140ms) creates beautiful, continuous radiant trails (nova rays)
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 140);
    }

    // Convert master HSV back to RGB to retrieve raw real-time frequency amplitudes (Bass, Mid, Treble)
    auto rgb = ::Utility::hsv2rgb(masterH, masterS, masterV);
    float bassAmp = static_cast<float>(rgb.r) / 255.0f;
    float midAmp  = static_cast<float>(rgb.g) / 255.0f;
    float trebAmp = static_cast<float>(rgb.b) / 255.0f;

    // Map the 3 real-time frequency amplitudes into 16 virtual FFT bins with organic jitter
    float fftResult[16];
    for (uint8_t bin = 0; bin < 16; bin++) {
        if (bin < 5) {
            // Bass bands (0..4) driven by Bass amplitude (Red channel)
            fftResult[bin] = bassAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        else if (bin < 11) {
            // Mid bands (5..10) driven by Mid amplitude (Green channel)
            fftResult[bin] = midAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        else {
            // Treble bands (11..15) driven by Treble amplitude (Blue channel)
            fftResult[bin] = trebAmp * (0.7f + static_cast<float>(random(0, 30)) / 100.0f);
        }
        if (fftResult[bin] > 1.0f) fftResult[bin] = 1.0f;
    }

    // Rotate the emitter nozzle slowly over time
    nozzleAngle += 0.06f;
    if (nozzleAngle >= 2.0f * 3.14159265f) {
        nozzleAngle -= 2.0f * 3.14159265f;
    }

    // Center point of the matrix
    float cx = (cfg.cols - 1) / 2.0f;
    float cy = (cfg.rows - 1) / 2.0f;

    // 1. EMIT NEW PARTICLES FROM THE CENTER (O(1) trig execution)
    static uint8_t spawnTrigger = 0;
    spawnTrigger++;

    // Emit particles every 3 frames for optimal balanced density
    if (spawnTrigger >= 3) {
        spawnTrigger = 0;

        for (uint8_t j = 0; j < 16; j++) {
            // Spawn only if the frequency bin amplitude crosses the threshold
            if (fftResult[j] > 0.35f && random(0, 100) < 40) {
                for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
                    if (!particles[p].active) {
                        particles[p].active = true;
                        particles[p].x = cx;
                        particles[p].y = cy;
                        particles[p].age = 1.0f;

                        // Distinct rainbow hue per frequency bin (radial color map)
                        particles[p].hue = (static_cast<float>(j) / 16.0f) * 0.85f;

                        // Modulate particle speed based on frequency amplitude and master LEVEL (masterV)
                        float speed = (0.10f + 0.30f * fftResult[j]) * masterV;

                        if (cfg.rows > 1) {
                            // 2D: Emit radially outward based on the current rotating angle
                            float emitAngle = nozzleAngle + (j * (2.0f * 3.14159265f / 16.0f));
                            particles[p].vx = cos(emitAngle) * speed;
                            particles[p].vy = sin(emitAngle) * speed;
                        } else {
                            // 1D: Alternate left and right horizontal propulsion from the center
                            particles[p].vx = (j % 2 == 0) ? speed : -speed;
                            particles[p].vy = 0.0f;
                        }
                        break; // Particle spawned for this bin, move to the next
                    }
                }
            }
        }
    }

    // 2. UPDATE AND RENDER ACTIVE PARTICLES
    float gravity = (cfg.rows > 1) ? 0.02f : 0.0f; // Soft gravity pull in 2D to create a weeping star effect
    for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
        if (particles[p].active) {
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;
            particles[p].vy += gravity;
            particles[p].age -= 0.03f; // Decay rate

            if (particles[p].age <= 0.0f ||
                particles[p].x < 0 || particles[p].x >= cfg.cols ||
                particles[p].y < 0 || particles[p].y >= cfg.rows) {
                particles[p].active = false;
                continue;
            }

            // Render active peak particle
            int px = static_cast<int>(round(particles[p].x));
            int py = static_cast<int>(round(particles[p].y));
            if (px >= 0 && px < cfg.cols && py >= 0 && py < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(px, py, cfg);
                if (idx < cfg.count) {
                    animatedColors[idx]->setColor(particles[p].hue, 1.0f, particles[p].age * masterV, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_PARTICLE_CENTER_GEQ_H
