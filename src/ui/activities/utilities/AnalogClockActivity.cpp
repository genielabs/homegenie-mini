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

#include "AnalogClockActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Utilities {

    void AnalogClockActivity::onResume() {

        canvas->setColorDepth(lgfx::color_depth_t::palette_4bit);

        if (clockBaseSprite == nullptr) {
            clockBaseSprite = new LGFX_Sprite(canvas);
            clockBaseSprite->setColorDepth(lgfx::palette_4bit);
        }
        if (hoursMinutesNeedle == nullptr) {
            hoursMinutesNeedle = new LGFX_Sprite(canvas);
            hoursMinutesNeedle->setColorDepth(lgfx::palette_4bit);
        }
        if (secondsNeedle == nullptr) {
            secondsNeedle = new LGFX_Sprite(canvas);
            secondsNeedle->setColorDepth(lgfx::palette_4bit);
        }

        clockBaseSprite->createSprite(diameter, diameter);
        hoursMinutesNeedle->createSprite(9, radius);
        secondsNeedle->createSprite(2, radius);

        // draw sprites

//        clockBaseSprite->setTextFont(2);
        clockBaseSprite->setTextDatum(lgfx::middle_center);

        clockBaseSprite->fillCircle(center, center, radius    , 3);
        clockBaseSprite->drawCircle(center, center, radius - 1, 15);

        for (int i = 1; i <= 60; ++i) {
            float rad = i * 6 * - 0.0174532925;
            float cosy = - cos(rad) * (center * 10 / 11);
            float sinx = - sin(rad) * (center * 10 / 11);
            bool flg = 0 == (i % 5);
            clockBaseSprite->fillCircle(center + sinx + 1, center + cosy + 1, flg * 3 + 1, 4);
            clockBaseSprite->fillCircle(center + sinx    , center + cosy    , flg * 3 + 1, 12);
            if (flg) {
                cosy = - cos(rad) * (center * 10 / 13);
                sinx = - sin(rad) * (center * 10 / 13);
                clockBaseSprite->setTextColor(1);
                clockBaseSprite->drawNumber(i / 5, center + sinx + 1, center + cosy + 4);
                clockBaseSprite->setTextColor(15);
                clockBaseSprite->drawNumber(i / 5, center + sinx    , center + cosy + 3);
            }
        }

        hoursMinutesNeedle->setPivot(4, 100);
        secondsNeedle->setPivot(1, 100);

        // outer
        hoursMinutesNeedle->fillTriangle(2, 24, 18, hoursMinutesNeedle->height() + 68, 0, hoursMinutesNeedle->height() + 68, 2);
        // inner
        hoursMinutesNeedle->fillTriangle(4, 24, 16, hoursMinutesNeedle->height() + 64, 0, hoursMinutesNeedle->height() + 64, 5);

        hoursMinutesNeedle->fillCircle(4, 100, 3, 15);
        hoursMinutesNeedle->drawCircle(4, 100, 4, 14);

        secondsNeedle->fillScreen(9);
        secondsNeedle->fillRect(0, 99, 3, 3, 8);
        secondsNeedle->drawFastVLine(1, 0, 119 - 8, 8);

    }
    void AnalogClockActivity::onPause() {
        clockBaseSprite->deleteSprite();
        hoursMinutesNeedle->deleteSprite();
        secondsNeedle->deleteSprite();
    }

    void AnalogClockActivity::onDraw()
    {
        auto rtc = Config::getRTC();
        float hrs = rtc->getHour();
        float min = rtc->getMinute();
        float sec = rtc->getSecond();
        float ms = rtc->getMillis();

        hrs = hrs + (min / 60.0f);
        min = min + (sec / 60.0f);
        sec = sec + (ms / 1000.0f);
    //    Serial.printf("%02.2f:%02.2f:%02.2f.%02.2f\n", hrs, min, sec, ms);
        if (hrs > 12) hrs -= 12;
        float deg_inc = 6.0f;

        drawDot((int)sec % 60, 14);
        drawDot((int)min % 60, 15);
        drawDot((((int)min/60)*5)%60, 15);

        // copy sprites to the display

        // hours needle
        hoursMinutesNeedle->pushRotateZoom(hrs * deg_inc * 5.0f, 1.0, 0.8, maskColor);
        // minute needle (using same sprite as for hours needle)
        hoursMinutesNeedle->pushRotateZoom(min * deg_inc , 1.0, 1.0, maskColor);
        // seconds needle
        secondsNeedle->pushRotateZoom(sec * deg_inc, 1.0, 1.0, maskColor);

//        canvas->pushRotateZoom(0, zoom, zoom, maskColor);
        clockBaseSprite->pushSprite(0, 0);
    }

    void AnalogClockActivity::drawDot(int pos, int palette)
    {
        bool flg = 0 == (pos % 5);
        float rad = pos * 6 * - 0.0174532925;
        float cos_y = - cos(rad) * (center * 10 / 11);
        float sin_x = - sin(rad) * (center * 10 / 11);
        canvas->fillCircle(center + sin_x, center + cos_y, flg * 3 + 1, palette);
    }

}}}

#endif //ENABLE_UI
