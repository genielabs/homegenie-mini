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
#ifndef HOMEGENIE_MINI_FX_VORTEX_H
#define HOMEGENIE_MINI_FX_VORTEX_H

#include "fx.h"

struct VortexParticle {
    float x, y;
    float vx, vy;
    float hue;
    float age;
    bool active;
};

inline void fx_vortex(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // 1. Retrieve all Master Color parameters (Hue, Saturation, Value)
    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();
    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f; // Completely off
    }

    const uint8_t MAX_PARTICLES = 48;
    static VortexParticle particles[MAX_PARTICLES];
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

    // Limit physics updates to 25ms (~40 FPS) for smooth spiral rendering
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    // Create a beautiful glowing spiral trail using master color parameters (smear-blur)
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 130);
    }

    // Center point of the matrix
    float cx = (cfg.cols - 1) / 2.0f;
    float cy = (cfg.rows - 1) / 2.0f;

    // Determine rotation direction based on master Hue (swaps rotation on melody changes)
    bool clockwise = (masterH > 0.5f);

    // Modulate nozzle rotation speed based on master value (physically dances with the music beats!)
    float rotationSpeed = (0.03f + (0.12f * masterV)) * (clockwise ? 1.0f : -1.0f);
    nozzleAngle += rotationSpeed;
    if (nozzleAngle >= 2.0f * 3.14159265f) {
        nozzleAngle -= 2.0f * 3.14159265f;
    }

    // 1. EMIT NEW PARTICLES (O(1) trig execution)
    uint8_t numArms = (cfg.rows > 1) ? 4 : 2; // 4 spiral arms for 2D, 2 directions for 1D
    static uint8_t spawnTrigger = 0;
    spawnTrigger++;

    // Spawn new particles from each nozzle arm every 3 frames for optimal density
    if (spawnTrigger >= 3) {
        spawnTrigger = 0;

        for (uint8_t j = 0; j < numArms; j++) {
            for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
                if (!particles[p].active) {
                    particles[p].active = true;
                    particles[p].x = cx;
                    particles[p].y = cy;
                    particles[p].age = 1.0f;

                    // Offset each arm's hue relative to the master Hue for visual separation
                    float armHue = masterH + ((float)j / (float)numArms);
                    if (armHue > 1.0f) armHue -= 1.0f;
                    particles[p].hue = armHue;

                    if (cfg.rows > 1) {
                        // Modulate particle speed based on master value (blasts outward on the beat!)
                        float speed = 0.10f + (0.25f * masterV);
                        float armAngle = nozzleAngle + (j * (2.0f * 3.14159265f / (float)numArms));
                        particles[p].vx = cos(armAngle) * speed;
                        particles[p].vy = sin(armAngle) * speed;
                    } else {
                        // 1D fallback: speed is also modulated by the master value
                        float speed = 0.15f + (0.20f * masterV);
                        particles[p].vx = (j == 0) ? speed : -speed;
                        particles[p].vy = 0.0f;
                    }
                    break; // Particle spawned for this arm, move to the next
                }
            }
        }
    }

    // 2. UPDATE AND RENDER ACTIVE PARTICLES
    for (uint8_t p = 0; p < MAX_PARTICLES; p++) {
        if (particles[p].active) {
            // Apply velocity vectors
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;

            // Cool down particle age (lifespan is ~1.25 seconds)
            particles[p].age -= 0.02f;

            // Deactivate if out of bounds or completely cooled down
            if (particles[p].age <= 0.0f ||
                particles[p].x < 0 || particles[p].x >= cfg.cols ||
                particles[p].y < 0 || particles[p].y >= cfg.rows) {
                particles[p].active = false;
                continue;
            }

            // Render active particle
            int px = (int)round(particles[p].x);
            int py = (int)round(particles[p].y);
            if (px >= 0 && px < cfg.cols && py >= 0 && py < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(px, py, cfg);
                if (idx < cfg.count) {
                    // Fully drive colors, saturation and brightness via master HSV (highly reactive for music mode)
                    animatedColors[idx]->setColor(particles[p].hue, masterS, particles[p].age * masterV, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_VORTEX_H
