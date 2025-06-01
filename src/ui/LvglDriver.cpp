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

#ifdef ENABLE_UI
#ifndef DISABLE_LVGL

#include "LvglDriver.h"

lv_color_t* LvglDriver::buffer0{};
lv_color_t* LvglDriver::buffer1{};
lv_disp_draw_buf_t LvglDriver::draw_buf;
LGFX_Device* LvglDriver::display_device{};
LGFX_Sprite* LvglDriver::gfx_canvas{};
lv_disp_drv_t LvglDriver::display_driver;
lv_indev_drv_t LvglDriver::indev_drv;
bool LvglDriver::initialized = false;
bool LvglDriver::disableInput = false;

void LvglDriver::begin(LGFX_Device *display) {
    if (initialized) {
        return;
    }

    display_device = display;

    uint16_t width = display_device->width();
    uint16_t height = display_device->height();

    uint32_t buffer_size_pixels = width * buffer_rows;
#ifdef BOARD_HAS_PSRAM
    size_t buffer_size_bytes = buffer_size_pixels * sizeof(lv_color_t);
    buffer0 = (lv_color_t*)ps_malloc(buffer_size_bytes);
    buffer1 = (lv_color_t*)ps_malloc(buffer_size_bytes);
#else
    buffer0 = new (std::nothrow) lv_color_t[buffer_size_pixels];
        buffer1 = new (std::nothrow) lv_color_t[buffer_size_pixels];
#endif

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

void LvglDriver::end() {
    if (buffer0) {
#ifdef BOARD_HAS_PSRAM
        free(buffer0);
#else
        delete[] buffer0;
#endif
        buffer0 = nullptr;
    }
    if (buffer1) {
#ifdef BOARD_HAS_PSRAM
        free(buffer1);
#else
        delete[] buffer1;
#endif
        buffer1 = nullptr;
    }
    gfx_canvas = nullptr;
    initialized = false;
    lv_deinit();
}

void LvglDriver::access_sd_card_start() {
    if (gfx_canvas == nullptr || !initialized) return;

    // Free the bus before accessing the SD card
    if (gfx_canvas->getStartCount() > 0) {
        gfx_canvas->endWrite();
    }
}

void LvglDriver::setCurrentCanvas(LGFX_Sprite *canvas) {
    gfx_canvas = canvas;
}

void LvglDriver::setRotation(lv_disp_rot_t rotation) {
    lv_disp_set_rotation(nullptr, rotation);
}

void LvglDriver::disp_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
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

void LvglDriver::touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (display_device == nullptr || !initialized || disableInput) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }

    uint16_t touchX, touchY;
    bool touched = display_device->getTouch(&touchX, &touchY);

    auto disp = lv_disp_get_default();
    if (touched) {
        data->state = LV_INDEV_STATE_PR;
        auto rotation = lv_disp_get_rotation(disp);
        auto panel_height = display_device->height();
        auto panel_width = display_device->width();
        switch (rotation) {
            case LV_DISP_ROT_NONE: {
                data->point.x = touchX;
                data->point.y = touchY;
            } break;
            case LV_DISP_ROT_90: {
                data->point.x = touchY;
                data->point.y = panel_width - 1 - touchX;
            } break;
            case LV_DISP_ROT_180: {
                data->point.x = panel_width - 1 - touchX;
                data->point.y = panel_height - 1 - touchY;
            } break;
            case LV_DISP_ROT_270: {
                data->point.x = panel_height - 1 - touchY;
                data->point.y = touchX;
            } break;
        }
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void LvglDriver::access_sd_card_end() {
    // not used
}

#endif // DISABLE_LVGL
#endif // ENABLE_UI
