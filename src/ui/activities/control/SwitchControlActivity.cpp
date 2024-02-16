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

#include "SwitchControlActivity.h"

#ifndef DISABLE_UI

namespace UI { namespace Activities { namespace Control {

    void SwitchControlActivity::attach(lgfx::LGFX_Device* displayDevice) {
        this->Activity::attach(displayDevice);
    }

    void SwitchControlActivity::onResume() {

        canvas->setColorDepth(lgfx::color_depth_t::palette_2bit);
        // text color
        canvas->setPaletteColor(1, LGFX_Sprite::color565(255, 255, 255));
        // accent color
        canvas->setPaletteColor(2, LGFX_Sprite::color565(40, 200, 40));
        // warn color
        canvas->setPaletteColor(3, LGFX_Sprite::color565(255, 20, 20));

        int hw = (int)round((float)canvas->width() / 2.0f);
        canvas->drawCircle(hw, hw, hw, 1);
        canvas->drawCircle(hw, hw, hw - 1, 1);

        if (toggleButton == nullptr) {
            // add input controls to this activity
            int toggleButtonRadius = (int)round((float)hw / 1.25f); // 96px on a 240w display
            toggleButton = new RoundButton(canvas, hw, hw, toggleButtonRadius);
// TODO: module->Name;
            toggleButton->setTitle("Light");
            addControl((InputControl*)toggleButton);
        }

        drawNavigationButtons();

    }

    void SwitchControlActivity::onPause() {

    }

    void SwitchControlActivity::onDraw() {
        if (toggleButton != nullptr) {

            if (showLoaderTs > 0) {
                setDrawInterval(0); // RT PRIORITY - loop
                float duration = 350.0f;
                float elapsed = millis() - showLoaderTs;
                if (elapsed <= duration) {
                    float p = AnimationHelper::animateFloat(0, 100, elapsed / duration, ANIMATION_EASING_LINEAR);
                    toggleButton->setProgress(p);
                } else {
                    showLoaderTs = 0;
                    toggleButton->setProgress(-1);
                    setDrawInterval(100); // 100ms loop interval
                }
            }
            // button color (red=3 for signaling error, green=2 for success)
            if (millis() - errorReportTs <= 500) {
                toggleButton->setColor(3);
            } else {
                toggleButton->setColor(2);
            }
            // button labels
            String level = "OFF";
            if (isSwitchedOn) {
                level = String(currentLevel) + "%";
            }
            toggleButton->setSubtitle(level.c_str());

        }
    }

}}}

#endif // DISABLE_UI
