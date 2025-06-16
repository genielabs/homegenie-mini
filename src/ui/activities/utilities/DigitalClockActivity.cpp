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

#include "DigitalClockActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Utilities {

    DigitalClockActivity::DigitalClockActivity() {
        setColorDepth(lgfx::palette_4bit);
    }

    void DigitalClockActivity::onStart() {
        if (display->width() < display->height()) {
            diameter = (float)display->width();
        } else {
            diameter = (float)display->height();
        }
        center = diameter / 2.0f;
        zoom = (float) (std::min(display->width(), display->height())) / diameter;
    }

    void DigitalClockActivity::onPause() {
        clockSprite->deleteSprite();
//        genieSprite->deleteSprite();
    }

    void DigitalClockActivity::onResume() {

        canvas->setPaletteColor(0, TFT_BLACK);
        canvas->setPaletteColor(1, TFT_WHITE);
        canvas->setPaletteColor(2, 30, 30, 30);
        canvas->setPaletteColor(3, TFT_GREENYELLOW);

        canvas->setPaletteColor(4, lgfx::color888(100, 0, 200));
        canvas->setPaletteColor(5, lgfx::color888(0, 255, 255));
        canvas->setPaletteColor(6, lgfx::color888(60, 60, 60));
        canvas->setPaletteColor(7, TFT_YELLOW);

        if (clockSprite == nullptr) {
            clockSprite = new LGFX_Sprite(canvas);
            clockSprite->setColorDepth(2);
        }

//        if (genieSprite == nullptr) {
//            genieSprite = new LGFX_Sprite(canvas);
//            genieSprite->setColorDepth(4);
//        }

        clockSprite->createSprite(140, 48);

//        genieSprite->createSprite(64, 64);
//        genieSprite->pushImageRotateZoom(0, 0, 0, 0, 0, 0.5, 0.5, 128, 128, genie_map, lgfx::color_depth_t::rgb565_2Byte, canvas->getPalette());
    }

    void DigitalClockActivity::onDraw()
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

        canvas->clear();


        // Current time clock digits
        clockSprite->setTextFont(7);
        clockSprite->setCursor(0, 0);
        clockSprite->setTextColor(PALETTE_COLOR_DIGIT_OFF);
        clockSprite->print("88:88");
        clockSprite->setCursor(0, 0);
        clockSprite->setTextColor(PALETTE_COLOR_DIGIT_ON);
        clockSprite->printf("%02d:%02d", (int)hrs, (int)min);


        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);

        // HomeGenie logos
        int32_t logoW = 151;
        int32_t logoH = 38;
        auto px = (int32_t)round(((float)canvas->width() - logoW) / 2.0f);
        auto py = (int32_t)round(((float)canvas->height() - logoH) / 2.0f);
//        canvas->drawBitmap(49, 133, logoHomeGenie, 151, 38, PALETTE_COLOR_DIGIT_OFF);
        canvas->drawBitmap(px, py + 40, logoHomeGenie, logoW, logoH, PALETTE_COLOR_TICK_CLEAR);
        canvas->drawBitmap(px, py + 38, logoHomeGenie, logoW, logoH, PALETTE_COLOR_HOMEGENIE);
//        genieSprite->pushSprite(96, 26);

        // Clock ticks
        for (int pos = 0; pos <= 59; pos++) {
            int dotSize = ((int)pos % 5) == 0 ? 4 : 1;
            drawDot((float)pos, dotSize, pos < sec+.25f ? PALETTE_COLOR_TICK_CLEAR : PALETTE_COLOR_TICK_OFF);
        }

        // Active tick cursor
        int dotSize = ((int)round(sec) % 5) == 0 ? 4 : 1;
        drawDot(round(sec), dotSize, PALETTE_COLOR_TICK_ON);

        clockSprite->pushSprite((canvas->width() - clockSprite->width()) / 2.0f, (int)canvas->getPivotY() - 48);

        // Render Pac Man =)) wakaa! waka!
        float rad_to_deg = 57.295779513082320876798154814105f;
        float rad = (sec == 0 ? 59.75f : sec - 0.25f) * 6.0f * - 0.0174532925f;
        float cos_y = - cos(rad) * ((float)center * 10 / 11.0f);
        float sin_x = - sin(rad) * ((float)center * 10 / 11.0f);
        float pacRotation = -(rad * rad_to_deg);
        canvas->fillEllipseArc(ox + (int)round(center + sin_x), oy + (int)round(center + cos_y), 0, 9, 0, 9, pacRotation + (pacAngle*18.0f/2), pacRotation - (pacAngle*18.0f/2), PALETTE_COLOR_PAC_MAN);
        if (pacAngle + pacAngleInc < 1 || pacAngle + pacAngleInc > 5) pacAngleInc *= -1;
        pacAngle += pacAngleInc;
    }

    void DigitalClockActivity::drawDot(float pos, int size, uint32_t palette)
    {
        // Offsets (x and y) to center things in the actual canvas
        auto ox = (int32_t)round(((float)canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round(((float)canvas->height() - diameter) / 2.0f);
        float rad = pos * 6.0f * - 0.0174532925f;
        float cos_y = - cos(rad) * ((float)center * 10 / 11.0f);
        float sin_x = - sin(rad) * ((float)center * 10 / 11.0f);
        canvas->fillCircle(ox + (int)round(center + sin_x), oy + (int)round(center + cos_y), size, palette);
    }

}}}

#endif //ENABLE_UI
