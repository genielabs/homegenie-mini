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

#ifndef HOMEGENIE_MINI_DIGITALCLOCK_H
#define HOMEGENIE_MINI_DIGITALCLOCK_H

#include "ui/Activity.h"

#ifdef ENABLE_UI

#include "ui/bitmaps/HomeGenieLogo.h"

#define PALETTE_COLOR_DIGIT_ON 1
#define PALETTE_COLOR_DIGIT_OFF 2
#define PALETTE_COLOR_TICK_OFF 4
#define PALETTE_COLOR_TICK_ON 5
#define PALETTE_COLOR_TICK_CLEAR 6
#define PALETTE_COLOR_PAC_MAN 7
#define PALETTE_COLOR_HOMEGENIE 7

namespace UI { namespace Activities { namespace Utilities {

    class DigitalClockActivity : public Activity {

    public:
        DigitalClockActivity() {
            setDrawInterval(10);  // Task.h - 100ms loop frequency
        }

        void attach(lgfx::LGFX_Device* displayDevice) override {
            this->Activity::attach(displayDevice);

            diameter = (float) display->width();
            center = diameter / 2.0f;
            zoom = (float) (std::min(display->width(), display->height())) / diameter;
        }

        void onResume() override;

        void onPause() override;

        void onDraw() override;

    private:
        LGFX_Sprite *clockSprite = nullptr;
//        LGFX_Sprite *genieSprite = nullptr;

        float diameter;
        float center;
//        int maskColor = TFT_TRANSPARENT; //lgfx::color888(0, 200, 0); // green screen =))
        float zoom;

        void drawDot(float pos, int size, uint32_t palette);
    };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_DIGITALCLOCK_H
