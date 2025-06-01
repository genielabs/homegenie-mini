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

#ifndef HOMEGENIE_MINI_LVGLACTIVITY_H
#define HOMEGENIE_MINI_LVGLACTIVITY_H

#include "Activity.h"

#ifdef ENABLE_UI

#ifndef DISABLE_LVGL

#include "ui/LvglDriver.h"

namespace UI {

    class LvglActivity: public Activity {

    public:
        LvglActivity() {
#ifdef BOARD_HAS_PSRAM
            setColorDepth(lgfx::rgb565_2Byte);
#else
            setColorDepth(lgfx::rgb332_1Byte);
#endif
            setDrawInterval(10); // Task.h
        }

        void onResume() override {
            if (activeScreen == nullptr) {
                activeScreen = lv_obj_create(nullptr);
            }
            lv_scr_load(activeScreen);
            LvglDriver::setCurrentCanvas(canvas);
        }
        void onPause() override {
            if (activeScreen) {
                lv_obj_clean(activeScreen);
                activeScreen = nullptr;
            }
        }
        void onDraw() override {
            lv_timer_handler();
        }

        bool isLvgl() override {
            return true;
        }

    protected:
        lv_obj_t* activeScreen = nullptr;

    private:

    };

}

#endif // DISABLE_LVGL

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_LVGLACTIVITY_H
