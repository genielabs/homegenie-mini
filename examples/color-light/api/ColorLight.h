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

using namespace Service;

class LightColor {
public:
    unsigned long duration;
    bool isAnimating = false;
    void setColor(float hue, float saturation, float value, unsigned long transitionMs) {
        duration = transitionMs;
        oh = h;
        os = s;
        ov = v;
        h = hue;
        s = saturation;
        v = value;
        startTime = millis();
        isAnimating = true;
    }
    float getProgress() {
        float p = (float)(millis() - startTime) / (float)duration;
        if (p >= 1) {
            isAnimating = false;
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
        auto orgb = Utility::hsv2rgb(hfix(oh), os, ov);
        auto crgb =  Utility::hsv2rgb(hfix(h), s, v);
        float r = orgb.r + ((crgb.r - orgb.r) * getProgress());
        return r;
    }
    float getGreen() {
        auto orgb = Utility::hsv2rgb(hfix(oh), os, ov);
        auto crgb =  Utility::hsv2rgb(hfix(h), s, v);
        float g = orgb.g + ((crgb.g - orgb.g) * getProgress());
        return g;
    }
    float getBlue() {
        auto orgb = Utility::hsv2rgb(hfix(oh), os, ov);
        auto crgb =  Utility::hsv2rgb(hfix(h), s, v);
        float b = orgb.b + ((crgb.b - orgb.b) * getProgress());
        return b;
    }
private:
    float h;
    float s;
    float v;
    float oh, os, ov;
    unsigned long startTime;
    float hfix(float h) {
        return 1.325f - h;
    }

};

class ColorLight: public Dimmer {
public:
    ColorLight(const char* domain, const char* address, const char* name);

    void loop() override;
    bool handleRequest(APIRequest*, ResponseCallback*) override;

    void onSetColor(std::function<void(float,float,float)> callback) {
        setColorCallback = std::move(callback);
    }
private:
    LightColor color;
    std::function<void(float,float,float)> setColorCallback = nullptr;

    void setColor(float h, float s, float v, float duration);
};


#endif //HOMEGENIE_MINI_COLORLIGHT_H
