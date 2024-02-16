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

#include "SensorValuesActivity.h"

void SensorValuesActivity::onResume() {
    canvas->setColorDepth(8);
}

void SensorValuesActivity::onPause() {
}

void SensorValuesActivity::onDraw()
{

    // Background image
    canvas->pushImageDMA(0, 0, 240, 240, bitmap_background_1, lgfx::color_depth_t::rgb565_2Byte, canvas->getPalette());
//    canvas->pushImageRotateZoom(0, 0, 0, 0, 0, 1, 1, 240, 240, bitmap_background_1, lgfx::color_depth_t::rgb565_2Byte, canvas->getPalette());

    canvas->setTextColor(TFT_WHITE);

    // Device name
    canvas->setFont(&fonts::Roboto_Thin_24);
    canvas->setTextColor(TFT_WHITE);
    canvas->drawCenterString(Config::getDeviceName(), (float)canvas->width() / 2.0f, 56);

    // Temperature
    canvas->setFont(&fonts::Font6);
    canvas->setCursor(52, 102);
    String temperatureValue = "--";
    auto temperature = this->sensorModule->getProperty(IOEventPaths::Sensor_Temperature);
    if (temperature != nullptr && temperature->value != nullptr) {
        canvas->printf("%.1f", *(float_t*)temperature->data);
    } else {
        canvas->print("--");
    }
    // Humidity
    canvas->setFont(&fonts::DejaVu24);
    canvas->setCursor(194, 112);
    auto humidity = this->sensorModule->getProperty(IOEventPaths::Sensor_Humidity);
    if (humidity != nullptr && humidity->value != nullptr) {
        canvas->printf("%.0f", *(float_t*)humidity->data);
    } else {
        canvas->print("--");
    }

    // Date and time
    canvas->setFont(&fonts::AsciiFont8x16);
    auto rtc = Config::getRTC();
    canvas->drawCenterString(rtc->getDate(), (float)canvas->width() / 2.0f, 165);
    canvas->setFont(&fonts::DejaVu24);
    canvas->drawCenterString(rtc->getTime(), (float)canvas->width() / 2.0f, 194);

}


