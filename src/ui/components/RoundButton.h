/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
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

#ifndef HOMEGENIE_MINI_ROUNDBUTTON_H
#define HOMEGENIE_MINI_ROUNDBUTTON_H

#include <LovyanGFX.hpp>

#include "ui/InputControl.h"

namespace UI { namespace Components {

    class RoundButton: InputControl {

    private:
        LGFX_Sprite* canvas;
        int cx, cy, w;
        float progress = -1;
        String title;
        String subtitle;
        int buttonColor = 2; // default color = green
        const lgfx::IFont* font = (lgfx::IFont*)&fonts::AsciiFont8x16;

    public:
        RoundButton(LGFX_Sprite* c, uint16_t centerX, uint16_t centerY, uint16_t width) {
            canvas = c;
            cx = centerX;
            cy = centerY;
            w = width;
        }


        bool hitTest(float x, float y) override {
            float r = ((float)w / 2.0f);
            return (x > cx - r && x < cx + r && y > cy - r && y < cy + r);
        }
        void draw() override {
            float r = ((float)w / 2.0f);
            canvas->fillCircle(cx, cy, r, buttonColor);
            canvas->fillCircle(cx, cy, r - 8, isPressed ? 1 : 0);
//            canvas->fillRect(cx-3, cy-12, 6, 24, isPressed ? 0 : 2);
            canvas->setFont(font);
            canvas->setTextColor(isPressed ? 0 : 1);
            canvas->drawCenterString(title, cx + 2, cy - (canvas->fontHeight() / 2) - 2);
            canvas->setTextColor(isPressed ? 0 : 2);
            canvas->drawCenterString(subtitle, cx + 2, cy + (canvas->fontHeight() /  2) + 2);

            if (progress >= 0 && progress <= 100) {
                float angle = progress * 3.6f;
                canvas->fillArc(cx, cy, 47, 42, angle - 24, angle + 24, 1);
            }
        }

        void setProgress(float p) {
            progress = p;
        }
        float getProgress() {
            return progress;
        }

        void setTitle(const char* s) {
            title = s;
        }
        void setSubtitle(const char* s) {
            subtitle = s;
        }

        void setColor(const int i) {
            buttonColor = i;
        }
        void setFont(const lgfx::IFont* f) {
            font = f;
        }
    };


}}

#endif //HOMEGENIE_MINI_ROUNDBUTTON_H
