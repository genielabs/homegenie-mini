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
        SystemInfoActivity() {
            setDrawInterval(200);  // Task.h - 200ms loop frequency
        }

        void attach(LGFX_Device* displayDevice) override {
            Activity::attach(displayDevice);

            if (display->width() < display->height()) {
                diameter = (float)display->width();
            } else {
                diameter = (float)display->height();
            }
            center = diameter / 2.0f;
            //zoom = (float) (std::min(display->width(), display->height())) / diameter;
        }

        void onResume() override;

        void onPause() override;

        void onDraw() override;

        void onTap(PointerEvent e) override {
            if (configDeviceButton != nullptr && e.target == configDeviceButton) {
                Preferences preferences;
                preferences.begin(CONFIG_SYSTEM_NAME, false);
                String systemMode = preferences.getString(CONFIG_KEY_system_mode);
                // toggle config mode flag
                preferences.putString(CONFIG_KEY_system_mode, systemMode.equals("config") ? "" : "config");
                preferences.end();
                meditate();
                esp_restart();
            }
            if (rotateScreenButton != nullptr && e.target == rotateScreenButton) {
                Preferences preferences;
                preferences.begin(CONFIG_SYSTEM_NAME, false);
                uint32_t displayRotation = preferences.getUInt(CONFIG_KEY_screen_rotation);
                if (displayRotation < 3) displayRotation++;
                else displayRotation = 0;
                // toggle config mode flag
                preferences.putUInt(CONFIG_KEY_screen_rotation, displayRotation);
                preferences.end();
                display->setRotation(displayRotation);
                // force re-allocation with new size
                pause();
                resume();
            }
        }
        void onSwipe(SwipeEvent e) override {}
        void onPan(PanEvent e) override {}
        void onTouch(PointerEvent e) override {}
        void onRelease(PointerEvent e) override {}


    private:
        RoundButton* configDeviceButton = nullptr;;
        RoundButton* rotateScreenButton = nullptr;;
        float center, diameter;
        enum ActivityColors {
            BACKGROUND,
            TEXT,
            ACCENT,
            WARN
        };

        void meditate() {
            // Offsets (x and y) to center things in the actual canvas
            auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
            auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

            display->clear();
            display->setFont(&lgfx::fonts::TomThumb);
            display->setTextColor(TFT_RED);
            display->setCursor(ox + 18, oy + 116);
            display->write("Software Success! (= Press left mouse button to continue.");
            display->setCursor(ox + 60, oy + 124);
            display->write("Guru Meditation #00000042.1010102A");
            delay(150);
            meditate_box(TFT_RED);
            delay(700);
            meditate_box(TFT_BLACK);
            delay(700);
            meditate_box(TFT_RED);
            delay(700);
            meditate_box(TFT_BLACK);
        }
        void meditate_box(const int color) {
            // Offsets (x and y) to center things in the actual canvas
            auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
            auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

            display->drawRect(ox + 2, oy + 104, 236, 36, color);
            display->drawRect(ox + 3, oy + 105, 234, 34, color);
            display->drawRect(ox + 4, oy + 106, 232, 32, color);
        }

    };

}}}

#endif //ENABLE_UI

#endif //HOMEGENIE_MINI_SYSTEMINFOACTIVITY_H
