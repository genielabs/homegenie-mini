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

#ifndef HOMEGENIE_MINI_ST7789_H
#define HOMEGENIE_MINI_ST7789_H

#ifdef ENABLE_UI

#include "Config.h"
#include "ui/DisplayDriver.h"

#include "input/Touch_CST816S_fix.hpp"

// default config

#ifdef ESP32_S3

    #define CONFIG_ST7789_DISPLAY_SCLK "39"
    #define CONFIG_ST7789_DISPLAY_MOSI "38"
    #define CONFIG_ST7789_DISPLAY_MISO "46"
    #define CONFIG_ST7789_DISPLAY_RST "0"

    #define CONFIG_ST7789_DISPLAY_DC "42"
    #define CONFIG_ST7789_DISPLAY_CS "45"

    #define CONFIG_ST7789_DISPLAY_BL "1"

    #define CONFIG_ST7789_TOUCH_PORT "0"
    #define CONFIG_ST7789_TOUCH_ADDRESS "21" // 0x15

    #define CONFIG_ST7789_TOUCH_INT "46"
    #define CONFIG_ST7789_TOUCH_SDA "48"
    #define CONFIG_ST7789_TOUCH_SCL "47"
    #define CONFIG_ST7789_TOUCH_RST "0"

#else

    #define CONFIG_ST7789_DISPLAY_SCLK "18"
    #define CONFIG_ST7789_DISPLAY_MOSI "23"
    #define CONFIG_ST7789_DISPLAY_MISO "19"
    #define CONFIG_ST7789_DISPLAY_RST "33"

    #define CONFIG_ST7789_DISPLAY_DC "27"
    #define CONFIG_ST7789_DISPLAY_CS "14"

    #define CONFIG_ST7789_DISPLAY_BL "32"

    #define CONFIG_ST7789_TOUCH_PORT "1"
    #define CONFIG_ST7789_TOUCH_ADDRESS "21"

    #define CONFIG_ST7789_TOUCH_INT "-1" /* -1 not used, or pin 35 */
    #define CONFIG_ST7789_TOUCH_SDA "25"
    #define CONFIG_ST7789_TOUCH_SCL "26"
    #define CONFIG_ST7789_TOUCH_RST "34"

#endif

namespace UI { namespace Drivers {

    using namespace Input;

    class ST7789 : public DisplayDriver {

    public:
        ST7789();
        LGFX_Device* getDisplay() override;

    private:
        lgfx::Panel_ST7789              _panel_instance;
        lgfx::Bus_SPI                   _bus_instance;
        //lgfx::Bus_I2C                 _bus_instance;
        //lgfx::Bus_Parallel8           _bus_instance;
        lgfx::Light_PWM                 _light_instance;
        //Touch_CST816S_fix               _touch_instance;
        lgfx::Touch_CST816S             _touch_instance;
        //lgfx::Touch_FT5x06           _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436
        //lgfx::Touch_GSL1680E_800x480 _touch_instance; // GSL_1680E, 1688E, 2681B, 2682B
        //lgfx::Touch_GSL1680F_800x480 _touch_instance;
        //lgfx::Touch_GSL1680F_480x272 _touch_instance;
        //lgfx::Touch_GSLx680_320x320  _touch_instance;
        //lgfx::Touch_GT911            _touch_instance;
        //lgfx::Touch_STMPE610         _touch_instance;
        //lgfx::Touch_TT21xxx          _touch_instance; // TT21100
        //lgfx::Touch_XPT2046          _touch_instance;
        LGFX_Device display;

    };

}}

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_ST7789_H
