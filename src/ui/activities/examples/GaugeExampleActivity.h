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
//     https://github.com/lovyan03/LovyanGFX/blob/master/examples/Sprite/RadgialGauge/RadgialGauge.ino

#ifndef HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H
#define HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H

#include "ui/Activity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Examples {

    class GaugeExampleActivity : public Activity {

    public:
        GaugeExampleActivity();
        void onResume() override;
        void onPause() override;
        void onDraw() override;

    private:
        LGFX_Sprite* base{};
        LGFX_Sprite* needle{};

        int32_t width = 239; // TODO: refactor as 'diameter'
        int32_t halfwidth = width >> 1; // TODO: refactor as 'radius'
        uint16_t transpalette{};
        float zoom{};

        float value{};
        bool invert = false;
        void draw();

    };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_GAUGEEXAMPLEACTIVITY_H
