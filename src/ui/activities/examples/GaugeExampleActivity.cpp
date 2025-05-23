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
 */

#include "GaugeExampleActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Examples {

    void GaugeExampleActivity::attach(LGFX_Device* displayDevice) {
        Activity::attach(displayDevice);
    }

    void GaugeExampleActivity::onResume() {

        canvas->setColorDepth(lgfx::palette_2bit);

        if (base == nullptr) {

            base = new LGFX_Sprite(canvas);
            needle = new LGFX_Sprite(canvas);
#ifdef BOARD_HAS_PSRAM
            base->setPsram(true);
            needle->setPsram(true);
#endif

            canvas->setColorDepth(lgfx::palette_2bit);
            base->setColorDepth(lgfx::palette_2bit);
            needle->setColorDepth(lgfx::palette_2bit);

        }

        base->createSprite(width, width);
        needle->createSprite(3, 11);

        int lw = std::min(canvas->width(), canvas->height());

        zoom = (float) lw / width;

//        int px = display->width() >> 1;
//        int py = display->height() >> 1;
//        display->setPivot(px, py);
/*
        display->setColorDepth(24);
        for (int i = 0; i < 180; i += 2) {
            display->setColor(display->color888(i * 1.4, i * 1.4 + 2, i * 1.4 + 4));
            display->fillArc(px, py, (lw >> 1), (lw >> 1) - zoom * 3, 90 + i, 92 + i);
            display->fillArc(px, py, (lw >> 1), (lw >> 1) - zoom * 3, 88 - i, 90 - i);
        }
        for (int i = 0; i < 180; i += 2) {
            display->setColor(display->color888(i * 1.4, i * 1.4 + 2, i * 1.4 + 4));
            display->fillArc(px, py, (lw >> 1) - zoom * 4, (lw >> 1) - zoom * 7, 270 + i, 272 + i);
            display->fillArc(px, py, (lw >> 1) - zoom * 4, (lw >> 1) - zoom * 7, 268 - i, 270 - i);
        }*/
//        display->setColorDepth(16);

        base->setFont(&fonts::Orbitron_Light_24);
//        base->setFont(&fonts::Roboto_Thin_24);

        base->setTextDatum(lgfx::middle_center);

        base->fillCircle(halfwidth, halfwidth, halfwidth - 8, 1);
        base->fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 11, 135, 45, 3);
        base->fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23, 2, 43, 2);
        base->fillArc(halfwidth, halfwidth, halfwidth - 20, halfwidth - 23, 317, 358, 2);
        base->setTextColor(3);

        for (int i = -5; i <= 25; ++i) {
            bool flg = 0 == (i % 5);
            if (flg) {
                base->fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 24, 179.8 + i * 9,
                              180.2 + i * 9, 3);
                base->fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 20, 179.4 + i * 9,
                              180.6 + i * 9, 3);
                base->fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 14, 179 + i * 9, 181 + i * 9,
                              3);
                float rad = i * 9 * 0.0174532925;
                float ty = -sin(rad) * (halfwidth * 10 / 15);
                float tx = -cos(rad) * (halfwidth * 10 / 17);
                base->drawFloat((float) i / 10, 1, halfwidth + tx, halfwidth + ty); // 数値描画
            } else {
                base->fillArc(halfwidth, halfwidth, halfwidth - 10, halfwidth - 17, 179.5 + i * 9,
                              180.5 + i * 9, 3);
            }
        }

        needle->setPivot(1, 9);
        needle->drawRect(0, 0, 3, 11, 2);

        canvas->setPaletteColor(0, 0, 0, 0);
        canvas->setPaletteColor(1, 0, 0, 120);
        canvas->setPaletteColor(2, 255, 81, 21);
        canvas->setPaletteColor(3, 255, 255, 191);


    }

    void GaugeExampleActivity::onPause() {

        // TODO: free resources

        base->deleteSprite();
        needle->deleteSprite();


    }

    bool invert = false;
    void GaugeExampleActivity::onDraw() {
        if (invert && value > 0) {
            value -= 0.25;
            draw();
            return;
        } else invert = false;

        if (1.9 > (value += 0.005 + value * 0.05)) {
            draw();
        } else if (value > 1.9) {
            invert = true;
        } else if ((-0.5 < (value -= 0.1))) {
            draw();
        } else if ((0.0 > (value += 0.05))) {
            draw();
        }
    }

}}}

#endif // ENABLE_UI
