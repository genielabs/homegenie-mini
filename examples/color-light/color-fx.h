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
 *
 */

#include <service/api/devices/ColorLight.h>

using namespace Service;
using namespace Service::API::devices;

#define FX_DEG_NORM(X) (((int) round((X) * 360.0f) % 360) / 360.0f)

typedef LightColor* AnimatedColor;
AnimatedColor* animatedColors = nullptr;

float hueOffset = 0;
float hueZoom = 1;

void fx_init(int count, LightColor& currentColor) {
    // Allocate "animated" colors
    animatedColors = new AnimatedColor[count];
    for (int i = 0; i < count; i++) {
        animatedColors[i] = new LightColor();
        animatedColors[i]->setColor(currentColor.getHue(), currentColor.getSaturation(), currentColor.getValue(), 0);
    }
}

void fx_reset(Adafruit_NeoPixel* pixels, LightColor& color) {
//    hueOffset = 0;
    hueZoom = 1;
}

void fx_solid(Adafruit_NeoPixel* pixels, LightColor& color, int transitionMs = 200) {
    if (pixels == nullptr) return;

    for (int i = 0; i < pixels->numPixels(); i++) {
        animatedColors[i]->setColor(color.getHue(), color.getSaturation(), color.getValue(), transitionMs);
    }
}

float currentSaturation;
float cursorDirection = 1;
unsigned long rainbow_refresh_ts = 0;
void fx_rainbow(Adafruit_NeoPixel* pixels, LightColor& color, float iterations = 1) {
    if (pixels == nullptr || millis() - rainbow_refresh_ts <= 100) return;

    uint numPixels = pixels->numPixels();
    currentSaturation = color.getSaturation();
    float length = numPixels * hueZoom;

    float hueStep = 1.0f / (float) length * iterations;

    float v = color.getValue();
    for (int i = 0; i < numPixels; i++) {
        float inc = (i < numPixels / iterations ? (float)i : (float)(numPixels - i)) * hueStep;
        float h = FX_DEG_NORM((color.getHue() + hueOffset) + inc);
        animatedColors[i]->setColor(h, currentSaturation, v, currentSaturation * 100);
    }

    hueOffset += hueStep;
    if (hueOffset > 1) FX_DEG_NORM(hueOffset);
    hueZoom += (0.05f * (float)cursorDirection);
    if (hueZoom > 3) {
        cursorDirection = -1;
    } else if (hueZoom < 1) {
        cursorDirection = 1;
        hueZoom = 1;
    }

    rainbow_refresh_ts = millis();
}


unsigned long stripe_refresh_ts = 0;
int stripe_delay = 10; // ms
unsigned int stripe_transition = 150; // ms
int stripe_step = 3;
float stripe_cycle = 0;
float stripe_previous_hue;

void fx_white_stripes(Adafruit_NeoPixel* pixels, LightColor& color, bool brightWhite = false) {
    if (pixels == nullptr || millis() - stripe_refresh_ts <= stripe_delay) return;

    uint numPixels = pixels->numPixels();
    int stripe_length = (int)round((float)numPixels / 5.0f);
    float shift;

    if (stripe_length <= stripe_step) stripe_length = stripe_step * 2;

    stripe_cycle += /*color.getValue() * */ ((float)stripe_length / (float)50);
    if (stripe_cycle >= stripe_length) stripe_cycle = 0;

    shift = ((float)stripe_length) + (stripe_cycle * cursorDirection);

    cursorDirection = (color.getHue() - stripe_previous_hue > 0) ? 1 : -1;
    stripe_previous_hue = color.getHue();

    for (int i = 0; i < numPixels; i++) {
        float v = color.getValue();
        float s = color.getSaturation();
        if ((int)round(i + shift) % stripe_length < stripe_step) {
            // draw stripe
            animatedColors[i]->setColor(0, 0, brightWhite && v > 0 ? 1 : v,
                                        (float)stripe_transition / numPixels);
        } else {
            // draw solid color
            animatedColors[i]->setColor(color.getHue(), s, v,
                                        (float)stripe_transition * v);
        }
    }

    stripe_refresh_ts = millis();
}


unsigned long kaleidoscope_refresh_ts = 0;
int kaleidoscope_delay = 500;

void fx_kaleidoscope(Adafruit_NeoPixel* pixels, LightColor& color) {
    if (pixels == nullptr || millis() - kaleidoscope_refresh_ts <= kaleidoscope_delay) return;

    for (int i = 0; i < pixels->numPixels(); i++) {
        float rnd1 = random(1000);
        float rnd2 = random(1000);
        if ((int)rnd1 % 2 == 0) {
            animatedColors[i]->setColor(color.getHue(), color.getSaturation(), color.getValue(), 300);
        } else {
            animatedColors[i]->setColor(rnd1 / 1000, rnd2 / 1000, color.getValue(), 500);
        }
    }

    kaleidoscope_refresh_ts = millis();
}
