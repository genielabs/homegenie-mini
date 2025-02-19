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

#include "RoundDisplay.h"

#ifdef ENABLE_UI

namespace UI { namespace Drivers {

    RoundDisplay::RoundDisplay() {
        {

            // SPI config
            {
                auto cfg = _bus_instance.config();
                cfg.pin_sclk = CONFIG_DISPLAY_SCLK;
                cfg.pin_mosi = CONFIG_DISPLAY_MOSI;
                cfg.pin_miso = CONFIG_DISPLAY_MISO;
                cfg.pin_dc   = CONFIG_DISPLAY_DC;
                _bus_instance.config(cfg);
                _panel_instance.setBus(&_bus_instance);
            }

            // LCD panel config
            {
                auto cfg = _panel_instance.config();

                cfg.pin_cs           =    CONFIG_DISPLAY_CS;
                cfg.pin_rst          =    CONFIG_DISPLAY_RST;
                //cfg.pin_busy         =    -1;
                cfg.panel_width      =   240;
                cfg.panel_height     =   240;
                //cfg.offset_x         =     0;
                //cfg.offset_y         =     0;
                //cfg.offset_rotation  =     0;
                //cfg.dummy_read_pixel =     12;
                //cfg.dummy_read_bits  =     1;
                cfg.readable         = true;
                cfg.invert           = true;
                cfg.rgb_order        = false;
                //cfg.dlen_16bit       = true;
                cfg.bus_shared       = false;

                cfg.memory_width     =   240;
                cfg.memory_height    =   240;

                _panel_instance.config(cfg);
            }

            // Backlight
            {
                auto cfg = _light_instance.config();

                cfg.pin_bl = CONFIG_DISPLAY_BL;
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
                cfg.x_max      = 239;
                cfg.y_min      = 0;
                cfg.y_max      = 239;
                cfg.pin_int    = CONFIG_TOUCH_INT;
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
                cfg.i2c_port = CONFIG_TOUCH_PORT;
                cfg.i2c_addr = CONFIG_TOUCH_ADDRESS;
                cfg.pin_sda  = CONFIG_TOUCH_SDA;
                cfg.pin_scl  = CONFIG_TOUCH_SCL;
                cfg.pin_rst  = CONFIG_TOUCH_RST;
                cfg.freq = 400000;

                _touch_instance.config(cfg);
                _panel_instance.setTouch(&_touch_instance);
            }

            display.setPanel(&_panel_instance);
            display.init();


            /*
            // Touch display calibration
            if (display.touch())
            {
                if (display.diameter() < display.height()) display.setRotation(display.getRotation() ^ 1);

                // Helper text
                display.setTextDatum(textdatum_t::middle_center);
                display.drawString("touch the arrow marker.", display.diameter()>>1, display.height() >> 1);
                display.setTextDatum(textdatum_t::top_left);

                // Touch the 4 arrows on the display to calibrate
                std::uint16_t fg = TFT_WHITE;
                std::uint16_t bg = TFT_BLACK;
                if (display.isEPD()) std::swap(fg, bg);
                display.calibrateTouch(nullptr, fg, bg, std::max(display.diameter(), display.height()) >> 3);
            }
            //*/
        }
    }

    lgfx::LGFX_Device* RoundDisplay::getDisplay() {
        return &display;
    };

}}

#endif // ENABLE_UI
