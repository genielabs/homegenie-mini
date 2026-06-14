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
#ifndef HOMEGENIE_MINI_RAINBOW_H
#define HOMEGENIE_MINI_RAINBOW_H

#include "fx.h"

#define FX_DEG_NORM(X) (((int) round((X) * 360.0f) % 360) / 360.0f)

float hueOffset = 0;
float hueZoom = 1;
float currentSaturation;

unsigned long rainbow_refresh_ts = 0;

inline void fx_rainbow(const MatrixConfig& cfg, const LightColor& color, float iterations = 1) {
    if (millis() - rainbow_refresh_ts <= 100) return;

    currentSaturation = color.getSaturation();
    float length = cfg.count * hueZoom;

    float hueStep = 1.0f / length * iterations;

    float v = color.getValue();
    for (int i = 0; i < cfg.count; i++) {
        float inc = (i < cfg.count / iterations ? (float)i : (float)(cfg.count - i)) * hueStep;
        float h = FX_DEG_NORM((color.getHue() + hueOffset) + inc);
        animatedColors[i]->setColor(h, currentSaturation, v, currentSaturation * 100);
    }

    hueOffset += hueStep;
    if (hueOffset > 1) FX_DEG_NORM(hueOffset);
    hueZoom += (0.05f * cursorDirection);
    if (hueZoom > 3) {
        cursorDirection = -1;
    } else if (hueZoom < 1) {
        cursorDirection = 1;
        hueZoom = 1;
    }

    rainbow_refresh_ts = millis();
}

#endif //HOMEGENIE_MINI_RAINBOW_H
