/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
 *
 */
#pragma once

#include <cmath>
#include <new>

#include <service/api/devices/ColorLight.h>

using namespace Service;
using namespace Service::API::devices;

typedef LightColor* AnimatedColor;
AnimatedColor* animatedColors = nullptr;

float cursorDirection = 1;

// The render loop owns this buffer. Prepare growth before releasing anything,
// retaining existing LightColor objects and their in-progress transitions.
uint16_t animatedColorCount = 0;
inline bool fx_resize(uint16_t count, const LightColor& currentColor) {
    if (count == animatedColorCount) return true;
    auto replacement = new (std::nothrow) AnimatedColor[count]();
    if (replacement == nullptr) return false;
    const uint16_t retained = count < animatedColorCount ? count : animatedColorCount;
    for (uint16_t i = 0; i < retained; ++i) replacement[i] = animatedColors[i];
    for (uint16_t i = retained; i < count; ++i) {
        replacement[i] = new (std::nothrow) LightColor();
        if (replacement[i] == nullptr) {
            for (uint16_t j = retained; j < i; ++j) delete replacement[j];
            delete[] replacement;
            return false;
        }
        replacement[i]->setColor(currentColor.getHue(), currentColor.getSaturation(), currentColor.getValue(), 0);
    }
    for (uint16_t i = count; i < animatedColorCount; ++i) delete animatedColors[i];
    delete[] animatedColors;
    animatedColors = replacement;
    animatedColorCount = count;
    return true;
}
inline bool fx_init(const MatrixConfig& cfg, const LightColor& currentColor) {
    return fx_resize(cfg.count, currentColor);
}

inline void fx_clear(const MatrixConfig& cfg, int transitionMs = 0) {
    for (int i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(1, 1, 0, transitionMs);
    }
}

inline void fx_solid(const MatrixConfig& cfg, const LightColor& color, int transitionMs = 200) {
    for (int i = 0; i < cfg.count; i++) {
        animatedColors[i]->setColor(color.getHue(), color.getSaturation(), color.getValue(), transitionMs);
    }
}
