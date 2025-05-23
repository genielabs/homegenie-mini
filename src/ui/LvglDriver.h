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

    static void setDisplay(LGFX_Device* display) {
        display_device = display;
    }

    static void begin(LGFX_Sprite* canvas) {
        if (initialized) {
            return;
        }

        gfx_canvas = canvas;
        uint16_t width = gfx_canvas->width();
        uint16_t height = gfx_canvas->height();

        uint32_t buffer_size_pixels = width * buffer_rows;
        buffer0 = new (std::nothrow) lv_color_t[buffer_size_pixels];
        buffer1 = new (std::nothrow) lv_color_t[buffer_size_pixels];

        lv_init();
        lv_disp_draw_buf_init(&draw_buf, buffer0, buffer1, buffer_size_pixels);

        lv_disp_drv_init(&display_driver);
        display_driver.hor_res = width;
        display_driver.ver_res = height;
        display_driver.flush_cb = disp_flush_cb;
        display_driver.draw_buf = &draw_buf;
        lv_disp_drv_register(&display_driver);

        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = touchpad_read_cb;
        lv_indev_drv_register(&indev_drv);

        initialized = true;
    }

    static void stop() {
        if (buffer0) {
            delete[] buffer0;
            buffer0 = nullptr;
        }
        if (buffer1) {
            delete[] buffer1;
            buffer1 = nullptr;
        }
        gfx_canvas = nullptr;
        initialized = false;
        lv_deinit();
    }


    static void access_sd_card_start() {
        if (gfx_canvas == nullptr || !initialized) return;

        // Free the bus before accessing the SD card
        if (gfx_canvas->getStartCount() > 0) {
            gfx_canvas->endWrite();
        }
    }
    static void access_sd_card_end() {
        // not used
    }

    static bool disableInput;

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

    static void disp_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
        if (gfx_canvas == nullptr || !initialized) return;

        if (gfx_canvas->getStartCount() == 0) {
            gfx_canvas->startWrite();
        }
        gfx_canvas->pushImageDMA(area->x1,
                                 area->y1,
                                  area->x2 - area->x1 + 1,
                                  area->y2 - area->y1 + 1,
                                 (lgfx::swap565_t*)&color_p->full);
        lv_disp_flush_ready(disp);
    }

    static void touchpad_read_cb(lv_indev_drv_t* indev_driver, lv_indev_data_t* data) {
        if (display_device == nullptr || !initialized || disableInput) {
            data->state = LV_INDEV_STATE_REL;
            return;
        }

        uint16_t touchX, touchY;
        bool touched = display_device->getTouch(&touchX, &touchY);

        if (touched) {
            data->state = LV_INDEV_STATE_PR;
            data->point.x = touchX;
            data->point.y = touchY;
        } else {
            data->state = LV_INDEV_STATE_REL;
        }
    }
};

#endif // DISABLE_LVGL
#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_LVGLDRIVER_H
