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
#ifndef HOMEGENIE_MINI_FX_WHITE_STRIPES_H
#define HOMEGENIE_MINI_FX_WHITE_STRIPES_H

#include "fx.h"

unsigned long stripe_refresh_ts = 0;
int stripe_delay = 10; // ms
unsigned int stripe_transition = 150; // ms
int stripe_step = 3;
float stripe_cycle = 0;
float stripe_previous_hue;

inline void fx_white_stripes(const MatrixConfig& cfg, const LightColor& color, bool brightWhite = false) {
    if (millis() - stripe_refresh_ts <= stripe_delay) return;

    int stripe_length = (int)round((float)cfg.count / 5.0f);
    float shift;

    if (stripe_length <= stripe_step) stripe_length = stripe_step * 2;

    stripe_cycle += /*color.getValue() * */ ((float)stripe_length / (float)50);
    if (stripe_cycle >= stripe_length) stripe_cycle = 0;

    shift = ((float)stripe_length) + (stripe_cycle * cursorDirection);

    cursorDirection = (color.getHue() - stripe_previous_hue > 0) ? 1 : -1;
    stripe_previous_hue = color.getHue();

    for (int i = 0; i < cfg.count; i++) {
        float v = color.getValue();
        float s = color.getSaturation();
        if ((int)round(i + shift) % stripe_length < stripe_step) {
            // draw stripe
            animatedColors[i]->setColor(0, 0, brightWhite && v > 0 ? 1 : v,
                                        (float)stripe_transition / cfg.count);
        } else {
            // draw solid color
            animatedColors[i]->setColor(color.getHue(), s, v,
                                        (float)stripe_transition * v);
        }
    }

    stripe_refresh_ts = millis();
}

#endif //HOMEGENIE_MINI_FX_WHITE_STRIPES_H
