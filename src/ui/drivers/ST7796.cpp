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

#include "ST7796.h"

namespace UI { namespace Drivers {

    ST7796::ST7796() {

        // SPI config
        {
            auto cfg = _bus_instance.config();
            cfg.pin_sclk = Config::getSetting("disp-sclk", CONFIG_ST7796_DISPLAY_SCLK).toInt();
            cfg.pin_mosi = Config::getSetting("disp-mosi", CONFIG_ST7796_DISPLAY_MOSI).toInt();
            cfg.pin_miso = Config::getSetting("disp-miso", CONFIG_ST7796_DISPLAY_MISO).toInt();
            cfg.pin_dc   = Config::getSetting("disp-dc", CONFIG_ST7796_DISPLAY_DC).toInt();
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // LCD panel config (ST7796)
        {
            auto cfg = _panel_instance.config();

            cfg.pin_cs           =    Config::getSetting("disp-cs", CONFIG_ST7796_DISPLAY_CS).toInt();
            cfg.pin_rst          =    Config::getSetting("disp-rst", CONFIG_ST7796_DISPLAY_RST).toInt();
            //cfg.pin_busy         =    -1;
            cfg.panel_width      =   320;
            cfg.panel_height     =   480;
            //cfg.offset_x         =     0;
            //cfg.offset_y         =     0;
            cfg.offset_rotation  =     2;
            //cfg.dummy_read_pixel =     8;
            //cfg.dummy_read_bits  =     1;
            cfg.readable         = false;
            cfg.invert           = true;
            cfg.rgb_order        = false;
            cfg.dlen_16bit       = false;
            cfg.bus_shared       = true;

            cfg.memory_width     =   320;
            cfg.memory_height    =   480;

            _panel_instance.config(cfg);
        }

        // Backlight
        {
            auto cfg = _light_instance.config();

            cfg.pin_bl = Config::getSetting("disp-bl", CONFIG_ST7796_DISPLAY_BL).toInt();
            //cfg.invert = false;
            //cfg.freq   = 44100;
            //cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        // Touch screen config
        {
            auto cfg = _touch_instance.config();

            cfg.x_min      = 0;
            cfg.x_max      = 319;
            cfg.y_min      = 0;
            cfg.y_max      = 479;
            cfg.bus_shared = true;
            cfg.offset_rotation = 0;

            // SPI
            //cfg.spi_host = VSPI_HOST;
            //cfg.freq = 1000000;
            //cfg.pin_sclk = 18;
            //cfg.pin_mosi = 23;
            //cfg.pin_miso = 19;
            //cfg.pin_cs   =  5;

            // I2C
            cfg.i2c_port = Config::getSetting("dstch-prt", CONFIG_ST7796_TOUCH_PORT).toInt();
            cfg.i2c_addr = Config::getSetting("dstch-adr", CONFIG_ST7796_TOUCH_ADDRESS).toInt();
            cfg.pin_sda  = Config::getSetting("dstch-sda", CONFIG_ST7796_TOUCH_SDA).toInt();
            cfg.pin_scl  = Config::getSetting("dstch-scl", CONFIG_ST7796_TOUCH_SCL).toInt();
            cfg.pin_rst  = Config::getSetting("dstch-rst", CONFIG_ST7796_TOUCH_RST).toInt();
            cfg.pin_int  = Config::getSetting("dstch-int", CONFIG_ST7796_TOUCH_INT).toInt();
            cfg.freq     = 400000;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        display.setPanel(&_panel_instance);
        display.init();
    }

    LGFX_Device* ST7796::getDisplay() {
        return &display;
    };

}}

#endif // ENABLE_UI
