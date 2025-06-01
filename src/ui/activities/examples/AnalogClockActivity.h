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

// Adapted from:
//     https://github.com/lovyan03/LovyanGFX/tree/master/examples/Sprite/ClockSample

#ifndef HOMEGENIE_MINI_ANALOGCLOCKACTIVITY_H
#define HOMEGENIE_MINI_ANALOGCLOCKACTIVITY_H

#include "ui/Activity.h"

#ifdef ENABLE_UI

#include "NTPClient.h"

namespace UI { namespace Activities { namespace Examples {

    class AnalogClockActivity: public Activity {
    public:
        AnalogClockActivity();

        void onStart() override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

    private:
        LGFX_Sprite* clockBaseSprite{};
        LGFX_Sprite* hoursMinutesNeedle{};
        LGFX_Sprite* secondsNeedle{};

        float diameter{};
        float radius{};
        int maskColor = TFT_TRANSPARENT;
        float zoom{};

        void drawDot(int pos, int palette);
    };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_ANALOGCLOCKACTIVITY_H
