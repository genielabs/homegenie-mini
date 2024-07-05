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
        void setColor(float hue, float saturation, float value, unsigned long transitionMs) {
            oh = getHue();
            os = getSaturation();
            ov = getValue();

            // constraints
            if (hue > 1) hue = 1;
            if (saturation > 1) saturation = 1;
            if (value > 1) value = 1;

            h = hue;
            s = saturation;
            v = value;

            duration = transitionMs;
            if (duration == 0) {
                oh = h;
                os = s;
                ov = v;
            }
            startTime = millis();
        }
        bool isAnimating() const {
            return (millis() - startTime) <= duration + 100;
        }
        float getProgress() const {
            float p = (float)(millis() - startTime) / ((float)duration + 1);
            return p < 1 ? p : 1;
        }
        float getHue() const {
            return oh + ((h - oh) * getProgress());
        }
        float getSaturation() const {
            return os + ((s - os) * getProgress());
        }
        float getValue() const {
            return ov + ((v - ov) * getProgress());
        }
        void setValue(float val) {
            v = val;
        }
        float getRed() const {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float r = (float)orgb.r + ((float)(crgb.r - orgb.r) * getProgress());
            return r;
        }
        float getGreen() const {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float g = (float)orgb.g + ((float)(crgb.g - orgb.g) * getProgress());
            return g;
        }
        float getBlue() const {
            auto orgb = Utility::hsv2rgb(hueFix(oh), os, ov);
            auto crgb =  Utility::hsv2rgb(hueFix(h), s, v);
            float b = (float)orgb.b + ((float)(crgb.b - orgb.b) * getProgress());
            return b;
        }
    private:
        float h = 0, s = 0, v = 0;
        float oh = 0, os = 0, ov = 0;
        unsigned long startTime = -1;
        unsigned long duration = 0;
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
