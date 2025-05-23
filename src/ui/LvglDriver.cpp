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

lv_color_t* LvglDriver::buffer0 = nullptr;
lv_color_t* LvglDriver::buffer1 = nullptr;
lv_disp_draw_buf_t LvglDriver::draw_buf;
LGFX_Device* LvglDriver::display_device = nullptr;
LGFX_Sprite* LvglDriver::gfx_canvas = nullptr;
lv_disp_drv_t LvglDriver::display_driver;
lv_indev_drv_t LvglDriver::indev_drv;
bool LvglDriver::initialized = false;
bool LvglDriver::disableInput = false;

#endif // DISABLE_LVGL
#endif // ENABLE_UI
