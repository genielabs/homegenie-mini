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
#ifndef HOMEGENIE_MINI_FX_FIREWORKS_H
#define HOMEGENIE_MINI_FX_FIREWORKS_H

#include "fx.h"

struct FxRocket {
    float x, y;
    float vx, vy;
    float hue;
    uint32_t fuseTime;
    uint32_t launchTime;
    bool active;
};

struct FxSpark {
    float x, y;
    float vx, vy;
    float hue;
    float brightness;
    bool active;
};

inline void fx_fireworks(const MatrixConfig& cfg, const LightColor& defaultColor) {
    if (cfg.cols == 0 || cfg.rows == 0) return;

    // Fixed pools to prevent runtime memory allocation and heap fragmentation
    const uint8_t MAX_ROCKETS = 2;
    const uint8_t MAX_SPARKS = 24;

    static FxRocket rockets[MAX_ROCKETS];
    static FxSpark sparks[MAX_SPARKS];
    static uint32_t lastUpdate = 0;
    static bool initialized = false;

    // Initialize pools on first boot
    if (!initialized) {
        for (uint8_t i = 0; i < MAX_ROCKETS; i++) {
            rockets[i].active = false;
        }
        for (uint8_t i = 0; i < MAX_SPARKS; i++) {
            sparks[i].active = false;
        }
        initialized = true;
    }

    // Limit physics updates to 30ms (~33 FPS) for consistent smooth rendering
    if (millis() - lastUpdate < 30) {
        return;
    }
    lastUpdate = millis();

    float masterV = defaultColor.getValue();
    if (masterV <= 0.01f) masterV = 1.0f;

    // Smooth motion blur by slowly fading all pixels to black over 100ms
    for (uint16_t i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(defaultColor.getHue(), defaultColor.getSaturation(), 0.0f, 100);
    }

    // Apply gravity only if we have a 2D matrix structure
    float gravity = (cfg.rows > 1) ? 0.04f : 0.0f;

    // 1. UPDATE AND RENDER ACTIVE ROCKETS
    for (uint8_t i = 0; i < MAX_ROCKETS; i++) {
        if (rockets[i].active) {
            // Apply horizontal and vertical physics
            rockets[i].x += rockets[i].vx;
            rockets[i].y += rockets[i].vy;
            rockets[i].vy += gravity; // Pulls rocket down

            int rx = static_cast<int>(round(rockets[i].x));
            int ry = static_cast<int>(round(rockets[i].y));

            // Draw rocket exhaust trail
            if (rx >= 0 && rx < cfg.cols && ry >= 0 && ry < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(rx, ry, cfg);
                if (idx < cfg.count) {
                    animatedColors[idx]->setColor(0.08f, 0.4f, masterV, 0); // Warm orange/white exhaust
                }
            }

            // Explode if fuse timed out, hit gravity apex, or reached boundaries
            bool exploded = (millis() - rockets[i].launchTime >= rockets[i].fuseTime) ||
                           (cfg.rows > 1 && rockets[i].vy >= 0.0f) ||
                           (rockets[i].x < 0 || rockets[i].x >= cfg.cols || rockets[i].y < 0 || rockets[i].y >= cfg.rows);

            if (exploded) {
                rockets[i].active = false;

                // Spawn explosion sparks
                uint8_t sparksToEmit = (cfg.rows > 1) ? 8 : 4; // Fewer sparks for 1D layout
                uint8_t spawned = 0;
                for (uint8_t s = 0; s < MAX_SPARKS; s++) {
                    if (!sparks[s].active) {
                        sparks[s].active = true;
                        sparks[s].x = rockets[i].x;
                        sparks[s].y = rockets[i].y;
                        sparks[s].hue = rockets[i].hue;
                        sparks[s].brightness = 1.0f;

                        if (cfg.rows > 1) {
                            // 2D: Generate a circular particle blast
                            float angle = (float)spawned * (2.0f * 3.14159265f / (float)sparksToEmit);
                            float speed = (float)random(10, 20) / 10.0f;
                            sparks[s].vx = cos(angle) * speed;
                            sparks[s].vy = sin(angle) * speed;
                        } else {
                            // 1D: Linear horizontal particle blast
                            sparks[s].vx = (spawned % 2 == 0) ? 1.0f : -1.0f;
                            sparks[s].vx *= (float)random(5, 15) / 10.0f;
                            sparks[s].vy = 0.0f;
                        }

                        spawned++;
                        if (spawned >= sparksToEmit) break;
                    }
                }
            }
        } else {
            // Relaunch dead rockets randomly
            if (random(0, 100) < 5) {
                rockets[i].active = true;
                rockets[i].launchTime = millis();
                rockets[i].fuseTime = random(800, 1800);
                rockets[i].hue = (float)random(0, 100) / 100.0f; // Random particle color

                if (cfg.rows > 1) {
                    // 2D: Shoot up from bottom center
                    rockets[i].x = random(cfg.cols / 4, (cfg.cols * 3) / 4);
                    rockets[i].y = cfg.rows - 1;
                    rockets[i].vx = (float)random(-10, 10) / 20.0f;  // Gentle horizontal wind drift
                    rockets[i].vy = (float)random(-50, -30) / 10.0f; // Initial vertical thrust
                } else {
                    // 1D: Shoot horizontally from a random edge towards the center
                    rockets[i].y = 0;
                    rockets[i].vy = 0;
                    if (random(0, 2) == 0) {
                        rockets[i].x = 0;
                        rockets[i].vx = (float)random(5, 15) / 10.0f; // Launch to the right
                    } else {
                        rockets[i].x = cfg.cols - 1;
                        rockets[i].vx = (float)random(-15, -5) / 10.0f; // Launch to the left
                    }
                }
            }
        }
    }

    // 2. UPDATE AND RENDER ACTIVE SPARKS
    for (uint8_t s = 0; s < MAX_SPARKS; s++) {
        if (sparks[s].active) {
            // Apply velocity and slight gravity drift to sparks
            sparks[s].x += sparks[s].vx;
            sparks[s].y += sparks[s].vy;
            sparks[s].vy += gravity * 0.5f;

            // Fade particle brightness over time
            sparks[s].brightness -= 0.06f;

            // Kill spark if faded out or went out of bounds
            if (sparks[s].brightness <= 0.0f ||
                sparks[s].x < 0 || sparks[s].x >= cfg.cols ||
                sparks[s].y < 0 || sparks[s].y >= cfg.rows) {
                sparks[s].active = false;
                continue;
            }

            int sx = (int)round(sparks[s].x);
            int sy = (int)round(sparks[s].y);
            if (sx >= 0 && sx < cfg.cols && sy >= 0 && sy < cfg.rows) {
                // Remap index based on physical layout configuration
                uint16_t idx = getLEDIndex(sx, sy, cfg);
                if (idx < cfg.count) {
                    animatedColors[idx]->setColor(sparks[s].hue, 1.0f, sparks[s].brightness * masterV, 0);
                }
            }
        }
    }
}

#endif //HOMEGENIE_MINI_FX_FIREWORKS_H
