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

#ifndef HOMEGENIE_MINI_COLORLIGHT_H
#define HOMEGENIE_MINI_COLORLIGHT_H

#include <HomeGenie.h>
#include <Utility.h>

#include "Dimmer.h"

namespace Service { namespace API { namespace devices {

    class LightColor {
    public:
        unsigned long duration = 400;
        void setColor(float hue, float saturation, float value, unsigned long transitionMs) {
            duration = transitionMs;
            if (duration <= 0) duration = 1;
            oh = h;
            os = s;
            ov = v;
            h = hue;
            s = saturation;
            v = value;
            startTime = millis();
        }
        bool isAnimating() const {
            return ((float)(millis() - startTime) / (float)duration) < 1;
        }
        float getProgress() const {
            float p = (float)(millis() - startTime) / (float)duration;
            if (p >= 1) {
                p = 1;
            }
            return p;
        }
        float getHue() {
            return oh + ((h - oh) * getProgress());
        }
        float getSaturation() {
            return os + ((s - os) * getProgress());
        }
        float getValue() {
            return ov + ((v - ov) * getProgress());
        }
        float getRed() {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float r = orgb.r + ((crgb.r - orgb.r) * getProgress());
            return r;
        }
        float getGreen() {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float g = orgb.g + ((crgb.g - orgb.g) * getProgress());
            return g;
        }
        float getBlue() {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float b = orgb.b + ((crgb.b - orgb.b) * getProgress());
            return b;
        }
    private:
        float h;
        float s;
        float v;
        float oh, os, ov;
        unsigned long startTime = -1;
        static float hueFix(float h) {
            return 1.325f - h;
        }

    };

    class ColorLight: public Dimmer {
    public:
        ColorLight(const char* domain, const char* address, const char* name);

        void loop() override;
        bool handleRequest(APIRequest*, ResponseCallback*) override;

        void onSetColor(std::function<void(LightColor)> callback) {
            setColorCallback = std::move(callback);
        }
    private:
        LightColor color;
        std::function<void(LightColor)> setColorCallback = nullptr;
    };

}}}

#endif //HOMEGENIE_MINI_COLORLIGHT_H
