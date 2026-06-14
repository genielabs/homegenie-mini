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
#ifndef HOMEGENIE_MINI_FX_METEOR_H
#define HOMEGENIE_MINI_FX_METEOR_H

#include "fx.h"

inline void fx_meteor(const MatrixConfig& cfg, const LightColor& color, int transitionMs = 300) {
    // Retrieve master value and enforce safe shut down if master is 0
    float v = color.getValue();
    if (v <= 0.01f) {
        v = 0.0f;
    }

    static int pos = 0;
    static int direction = 1;
    static uint32_t lastUpdate = 0;

    // Restrict position updates to a smooth 60ms rate
    if (millis() - lastUpdate < 60) {
        return;
    }
    lastUpdate = millis();

    // Cache base color parameters outside the loop
    float h = color.getHue();
    float s = color.getSaturation();

    // Calculate dynamic trail duration based on master value (brightness)
    // Minimum trail: transitionMs * 2.5 (approx 12 pixels of tail)
    // Maximum trail: 20% of the total cfg.count (proportional fade time)
    float minTrailTime = static_cast<float>(transitionMs) * 2.5f;
    float maxTrailTime = static_cast<float>(cfg.count) * 0.20f * 60.0f;

    // Safety check to prevent negative scaling on extremely short strips
    if (maxTrailTime < minTrailTime) {
        maxTrailTime = minTrailTime;
    }

    // Linearly interpolate trail duration between min and max based on master value (v)
    uint32_t dynamicTrailTime = (uint32_t)(minTrailTime + ((maxTrailTime - minTrailTime) * v));

    // O(1) OPTIMIZATION: Start fading out only the previously active pixel.
    // The trail duration is dynamically scaled, automatically adjusting the physical tail length!
    if (pos >= 0 && pos < cfg.count) {
        animatedColors[pos]->setColor(h, s, 0.0f, dynamicTrailTime);
    }

    // Move active pixel pointer
    pos += direction;
    if (pos >= cfg.count - 1) {
        pos = cfg.count - 1;
        direction = -1;
    } else if (pos <= 0) {
        pos = 0;
        direction = 1;
    }

    // Set the new active pixel to target brightness instantly
    if (pos >= 0 && pos < cfg.count) {
        animatedColors[pos]->setColor(h, s, v, 50);
    }
}

#endif //HOMEGENIE_MINI_FX_METEOR_H
