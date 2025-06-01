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

#ifndef HOMEGENIE_MINI_LVGLDRIVER_H
#define HOMEGENIE_MINI_LVGLDRIVER_H

#ifdef ENABLE_UI
#ifndef DISABLE_LVGL

#include <LovyanGFX.hpp>
#include <lvgl.h>

class LvglDriver {
public:
    LvglDriver() = delete;
    ~LvglDriver() = delete;
    LvglDriver(const LvglDriver&) = delete;
    LvglDriver& operator=(const LvglDriver&) = delete;

    static bool disableInput;

    static void begin(LGFX_Device* display);
    static void end();

    static void access_sd_card_start();
    static void access_sd_card_end();

    static void setCurrentCanvas(LGFX_Sprite* canvas);
    static void setRotation(lv_disp_rot_t rotation);

private:
    static const uint16_t buffer_rows = 60;

    static lv_disp_draw_buf_t draw_buf;

    static lv_color_t* buffer0;
    static lv_color_t* buffer1;

    static LGFX_Device* display_device;
    static LGFX_Sprite* gfx_canvas;

    static lv_disp_drv_t display_driver;
    static lv_indev_drv_t indev_drv;

    static bool initialized;

    static void disp_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
    static void touchpad_read_cb(lv_indev_drv_t* indev_driver, lv_indev_data_t* data);
};

#endif // DISABLE_LVGL
#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_LVGLDRIVER_H
