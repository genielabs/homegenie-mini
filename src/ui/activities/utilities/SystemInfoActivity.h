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

#ifndef HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H
#define HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H

#include "HomeGenie.h"
#include "ui/Activity.h"

#ifdef ENABLE_UI

#include "ui/components/RoundButton.h"
#include "ui/bitmaps/HomeGenieLogo.h"

namespace UI  { namespace Activities { namespace Utilities {

    using namespace Service;
    using namespace UI::Components;

    class SystemInfoActivity: public Activity {
    public:
        SystemInfoActivity();

        void onStart() override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void onTap(PointerEvent e) override;
        void onSwipe(SwipeEvent e) override {}
        void onPan(PanEvent e) override {}
        void onTouch(PointerEvent e) override {}
        void onRelease(PointerEvent e) override {}

    private:
        RoundButton* configDeviceButton{};
        RoundButton* rotateScreenButton{};
        float center{};
        float diameter{};

        enum ActivityColors {
            BACKGROUND,
            TEXT,
            ACCENT,
            WARN
        };

        void meditate();
        void meditate_box(const int color);

    };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H
