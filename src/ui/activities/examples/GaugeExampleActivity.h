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

#ifndef HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H
#define HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H

#include "ui/Activity.h"

#ifndef DISABLE_UI

namespace UI { namespace Activities { namespace Examples {

    class GaugeExampleActivity : public Activity {

    public:
        GaugeExampleActivity() {
            setDrawInterval(33); // Task.h
        }
        void attach(lgfx::LGFX_Device* displayDevice) override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

    private:
        LGFX_Sprite* base = nullptr;
        LGFX_Sprite* needle;

        int32_t width = 239;
        int32_t halfwidth = width >> 1;
        uint16_t transpalette = 0;
        float zoom;

        float value = 0;
        void draw()
        {
            base->pushSprite(0, 0);
            canvas->fillCircle(halfwidth, halfwidth, 7, 3);
            if (value >= 1.5f) {
                canvas->fillCircle(display->width() >> 1, (display->height() >> 1) + width * 4 / 10, 5, 2);
            }

            float angle = 270 + value * 90.0f;
            needle->pushRotateZoom(canvas->getPivotX() + drawOffset.x, canvas->getPivotY(), angle, 3.0f, 10.0f, transpalette); // 針をバッファに描画する

//            canvas->pushRotateZoom(0, zoom, zoom);
        }

    };

}}}

#endif //DISABLE_UI

#endif //HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H
