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
#ifndef HOMEGENIE_MINI_FX_WATERFALL_H
#define HOMEGENIE_MINI_FX_WATERFALL_H

#include "fx.h"

struct WaterParticle {
    float x, y;
    float vx, vy;
    float hue;
    float sat;
    float age; // 1.0 (spawned) down to 0.0 (dead)
    float fadeSpeed;
    bool active;
};

inline void fx_waterfall(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Retrieve master brightness and enforce safe shut down if master is 0
    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) {
        masterV = 0.0f;
    }

    uint16_t maxParticles = static_cast<uint16_t>(cfg.rows * cfg.cols * 0.6f);
    if (maxParticles == 0) maxParticles = 1;

    static std::vector<WaterParticle> particles;
    static uint32_t lastUpdate = 0;
    static bool initialized = false;

    if (!initialized || particles.size() != maxParticles) {
        particles.resize(maxParticles);
        for (uint16_t i = 0; i < maxParticles; i++) {
            particles[i].active = false;
        }
        initialized = true;
    }

    // Limit physics updates to 25ms (~40 FPS) for smooth fluid motion
    if (millis() - lastUpdate < 25) {
        return;
    }
    lastUpdate = millis();

    float masterH = defaultColor.getHue();
    float masterS = defaultColor.getSaturation();

    // High motion blur (180ms) to merge falling droplets into a solid liquid stream
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(masterH, masterS, 0.0f, 180);
    }

    float gravity = (cfg.rows > 1) ? 0.08f : 0.0f;

    // Slow horizontal nozzle wave (20-second back-and-forth cycle)
    float wave = (sin((millis() / 20000.0f) * 2.0f * 3.14159265f) + 1.0f) / 2.0f;
    float minX = cfg.cols / 3.0f;
    float maxX = cfg.cols - minX;
    float nozzleCenter = minX + (wave * (maxX - minX));

    // Spout width (3 pixels wide) (underflow and overflow protection)
    int16_t tempStart = (int16_t)round(nozzleCenter - 1.0f);
    int16_t tempEnd = (int16_t)round(nozzleCenter + 1.0f);

    uint16_t nozzleStart = (tempStart < 0) ? 0 : (uint16_t)tempStart;
    uint16_t nozzleEnd = (tempEnd >= cfg.cols) ? (cfg.cols - 1) : (uint16_t)tempEnd;

    // 3. EMIT NEW WATER DROPLETS
    if (random(0, 100) < 55) {
        // Emit 1 or 2 droplets at once for a more voluminous, copious vertical stream
        uint8_t dropletsToSpawn = random(1, 3);

        for (uint8_t s = 0; s < dropletsToSpawn; s++) {
            int spawnIdx = -1;

            // Search for an inactive particle slot in the pool (fino a maxParticles)
            for (uint16_t p = 0; p < maxParticles; p++) {
                if (!particles[p].active) {
                    spawnIdx = p;
                    break;
                }
            }

            // RESOURCE RECYCLING: If the pool is full, recycle the oldest pooled droplet at the bottom.
            if (spawnIdx == -1) {
                float oldestAge = 1.1f;
                int recycleIdx = -1;
                for (uint16_t p = 0; p < maxParticles; p++) {
                    if (particles[p].active && (cfg.rows == 1 || particles[p].y >= cfg.rows - 2)) {
                        if (particles[p].age < oldestAge) {
                            oldestAge = particles[p].age;
                            recycleIdx = p;
                        }
                    }
                }
                if (recycleIdx != -1) {
                    spawnIdx = recycleIdx;
                }
            }

            // Emit the droplet if an active or recycled slot was secured
            if (spawnIdx != -1) {
                particles[spawnIdx].active = true;
                particles[spawnIdx].age = 1.0f;

                // Add slight organic variations around master color for a shimmering water texture
                particles[spawnIdx].hue = masterH + (static_cast<float>(random(-15, 15)) / 360.0f);
                if (particles[spawnIdx].hue > 1.0f) particles[spawnIdx].hue -= 1.0f;
                if (particles[spawnIdx].hue < 0.0f) particles[spawnIdx].hue += 1.0f;

                particles[spawnIdx].sat = masterS * (0.8f + static_cast<float>(random(0, 20)) / 100.0f);
                if (particles[spawnIdx].sat > 1.0f) particles[spawnIdx].sat = 1.0f;

                if (cfg.rows > 1) {
                    // 2D: Emit from the oscillating nozzle at the top row (y = 0)
                    particles[spawnIdx].x = random(nozzleStart, nozzleEnd + 1);
                    particles[spawnIdx].y = 0.0f;
                    particles[spawnIdx].vx = static_cast<float>(random(-5, 5)) / 100.0f;
                    particles[spawnIdx].vy = static_cast<float>(random(15, 30)) / 100.0f;
                    particles[spawnIdx].fadeSpeed = static_cast<float>(random(10, 25)) / 1000.0f;
                } else {
                    // 1D: Emit from the drifting nozzle center
                    particles[spawnIdx].x = nozzleCenter;
                    particles[spawnIdx].y = 0.0f;
                    particles[spawnIdx].vx = (random(0, 2) == 0 ? 1.0f : -1.0f) * (static_cast<float>(random(10, 25)) / 100.0f);
                    particles[spawnIdx].vy = 0.0f;
                    particles[spawnIdx].fadeSpeed = static_cast<float>(random(15, 35)) / 1000.0f;
                }
            }
        }
    }

    // 4. UPDATE AND RENDER PARTICLES (fino a maxParticles)
    for (uint16_t p = 0; p < maxParticles; p++) {
        if (particles[p].active) {
            // Apply velocity vectors and gravity acceleration
            particles[p].x += particles[p].vx;
            particles[p].y += particles[p].vy;
            particles[p].vy += gravity;

            // Slowly reduce particle age
            particles[p].age -= particles[p].fadeSpeed;

            // 2D Ground Collision (Splash & Dynamic Split Flow)
            if (cfg.rows > 1 && particles[p].y >= cfg.rows - 1) {
                // Stack droplets organically on the bottom two rows (35% chance of stack)
                particles[p].y = cfg.rows - 1 - (random(0, 100) < 35 ? 1 : 0);
                particles[p].vy = -particles[p].vy * 0.10f; // Strongly dampened upward splash bounce

                // Split flow: particles flow left if on left side, right if on right side
                float flowDirection = (random(0, 100) < 50) ? -1.0f : 1.0f;

                // Slow down horizontal drainage to aggregate water closely and prevent gaps
                particles[p].vx = flowDirection * (static_cast<float>(random(3, 10)) / 100.0f);

                // Retain pooled water at the bottom significantly longer
                particles[p].fadeSpeed = 0.002f;
            }

            // Deactivate if out of bounds or faded out
            if (particles[p].age <= 0.0f ||
                particles[p].x < 0 || particles[p].x >= cfg.cols ||
                particles[p].y < 0 || particles[p].y >= cfg.rows) {
                particles[p].active = false;
                continue;
            }

            // Render active water droplet
            int px = static_cast<int>(round(particles[p].x));
            int py = static_cast<int>(round(particles[p].y));
            if (px >= 0 && px < cfg.cols && py >= 0 && py < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(px, py, cfg);
                if (idx < cfg.count) {
                    animatedColors[idx]->setColor(particles[p].hue, particles[p].sat, particles[p].age * masterV, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_WATERFALL_H