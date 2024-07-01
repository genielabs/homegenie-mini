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
 *
 */

#include <service/api/devices/ColorLight.h>

using namespace Service;
using namespace Service::API::devices;

typedef LightColor* AnimatedColor;
AnimatedColor* animatedColors = nullptr;

float currentSaturation;
float hueOffset = 0;
float hueRange = 1;
float cursorDirection = 1;

void fx_reset(Adafruit_NeoPixel* pixels, LightColor& color) {
//    hueOffset = 0;
    hueRange = 1;
}

void fx_solid(Adafruit_NeoPixel* pixels, LightColor& color) {
    float r = color.getRed();
    float g = color.getGreen();
    float b = color.getBlue();
    for (int i = 0; i < pixels->numPixels(); i++) {
        pixels->setPixelColor(i, r, g, b);
//        animatedColors[i]->setColor(color.getHue(), color.getSaturation(), color.getValue(), 0);
    }
}

void fx_rainbow(Adafruit_NeoPixel* pixels, LightColor& color) {
    if (color.getSaturation() > 1) {
        return;
        // TODO: ...
        currentSaturation = 1;
    } else {
        currentSaturation = color.getSaturation();
    }

    if (pixels != nullptr) {
        float h = color.getHue() + hueOffset;
        if (h > 1) h = ((int)round(h * 10000) % 10000) / 10000.0f;
        float hueStep = 1.0f / (float) pixels->numPixels();
        hueStep /= hueRange;
        float v = color.getValue();
        for (int i = 0; i < pixels->numPixels(); i++) {
            h += hueStep;
            auto rgb = Utility::hsv2rgb(h, currentSaturation, v);
            pixels->setPixelColor(i, rgb.r, rgb.g, rgb.b);
            animatedColors[i]->setColor(h, currentSaturation, v, 0);
        }
    }

    // animate
    hueOffset += (0.128f / pixels->numPixels());
    if (hueOffset > 1) hueOffset = 0;
    hueRange += (1.5f / pixels->numPixels()) * cursorDirection;
    if (hueRange > 5) {
        cursorDirection *= -1;
        hueRange = 5;
    } else if (hueRange < 1) {
        cursorDirection *= -1;
        hueRange = 1;
    }
}


unsigned long stripe_refresh_ts = 0;
int stripe_delay = 200; // ms
unsigned long stripe_transition = 200; // ms
int stripe_step = 3;
int stripe_length = 9;
int shift = 0;

void fx_white_stripes(Adafruit_NeoPixel* pixels, LightColor& color) {

    // animate
    if (millis() - stripe_refresh_ts > stripe_delay) {

        shift = shift % stripe_length;

        if (pixels != nullptr) {
            for (int i = 0; i < pixels->numPixels(); i++) {
                if ((i + shift) % stripe_length < stripe_step) {
                    // draw stripe
                    animatedColors[i]->setColor(0, 0, color.getValue(), stripe_transition);
                } else {
                    // draw solid color
                    animatedColors[i]->setColor(color.getHue(), color.getSaturation(),
                                                         color.getValue(), stripe_transition);
                }
            }
        }


        shift++;
        if (shift >= stripe_length) shift = 0;
        stripe_refresh_ts = millis();
    }

    // render
    if (pixels != nullptr) {
        for (int i = 0; i < pixels->numPixels(); i++) {
            pixels->setPixelColor(i, animatedColors[i]->getRed(), animatedColors[i]->getGreen(),
                                  animatedColors[i]->getBlue());
        }
    }

}


unsigned long kaleidoscope_refresh_ts = 0;
int kaleidoscope_delay = 500;

void fx_kaleidoscope(Adafruit_NeoPixel* pixels, LightColor& color) {

    // animate
    if (millis() - kaleidoscope_refresh_ts > kaleidoscope_delay) {
        for (int i = 0; i < pixels->numPixels(); i++) {
            float rnd1 = random(1000);
            float rnd2 = random(1000);
            animatedColors[i]->setColor(rnd1 / 1000, rnd2 / 1000, color.getValue(), 500);
        }

        kaleidoscope_refresh_ts = millis();
    }

    // render
    if (pixels != nullptr) {
        for (int i = 0; i < pixels->numPixels(); i++) {
            animatedColors[i]->setValue(color.getValue());
            pixels->setPixelColor(i, animatedColors[i]->getRed(), animatedColors[i]->getGreen(),
                                  animatedColors[i]->getBlue());
        }
    }

}
