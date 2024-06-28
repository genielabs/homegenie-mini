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

#include <HomeGenie.h>
#include <service/api/devices/ColorLight.h>

#include <Adafruit_NeoPixel.h>

#include "configuration.h"


using namespace Service;
using namespace Service::API::devices;

HomeGenie* homeGenie;

// Optional RGB Status LED
Adafruit_NeoPixel* statusLED = nullptr;

// LED strip/array
int count = 0; int pin = -1;
Adafruit_NeoPixel* pixels = nullptr;

bool changed = false;
unsigned long lastRefreshTs = 0;

// LED Blink callback when statusLED is configured
void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(1, 1, 0));
    } else {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    statusLED->show();
}

bool isConfigured = false;
float currentSaturation;
float hueOffset = 0;
float hueRange = 1;
float cursorDirection = 1;
LightColor currentColor;

ColorLight* mainModule;
ColorLight* rainbowModule;

void fx_reset(LightColor& color) {
//    hueOffset = 0;
    hueRange = 1;
    if (color.getSaturation() > 1) {
        // TODO: ...
        currentSaturation = 1;
    } else {
        currentSaturation = color.getSaturation();
    }
}

void fx_main(LightColor& color) {
    float r = color.getRed();
    float g = color.getGreen();
    float b = color.getBlue();
    if (statusLED != nullptr) {
        statusLED->setPixelColor(0, r, g, b);
    }
    if (pixels != nullptr) {
        for (int i = 0; i < count; i++) {
            pixels->setPixelColor(i, r, g, b);
        }
    }
}

void fx_rainbow(LightColor& color) {
    if (statusLED != nullptr) {
        statusLED->setPixelColor(0, color.getRed(), color.getGreen(), color.getBlue());
    }

    if (pixels != nullptr) {
        float h = color.getHue() + hueOffset;
        if (h > 1) h = ((int)round(h * 10000) % 10000) / 10000.0f;
        float hueStep = 1.0f / (float) count;
        hueStep /= hueRange;
        float v = color.getValue();
        for (int i = 0; i < count; i++) {
            h += hueStep;
            auto rgb = Utility::hsv2rgb(h, currentSaturation, v);
            pixels->setPixelColor(i, rgb.r, rgb.g, rgb.b);
        }
    }

    // animate
    hueOffset += (0.128f / count);
    if (hueOffset > 1) hueOffset = 0;
    hueRange += (1.5f / count) * cursorDirection;
    if (hueRange > 5) {
        cursorDirection *= -1;
        hueRange = 5;
    } else if (hueRange < 1) {
        cursorDirection *= -1;
        hueRange = 1;
    }
}

void refresh() {
    if (statusLED != nullptr) {
        statusLED->show();
    }
    if (pixels != nullptr) {
        pixels->show();
    }
}

void setup() {
    homeGenie = HomeGenie::getInstance();

    // Get status LED config
    int statusLedPin = Config::getSetting("stld-pin", "-1").toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
    }

    isConfigured = Config::isDeviceConfigured();
    if (!isConfigured) {

        // Custom status led (builtin NeoPixel RGB on pin 10)
        if (statusLED != nullptr) {
            Config::statusLedCallback(&statusLedCallback);
        }

    } else {

        // Get LED strip config
        count = Config::getSetting("leds-cnt").toInt();
        pin = (int16_t)Config::getSetting("leds-pin").toInt();
        if (count > 0 && pin >= 0) {
            auto pixelsType = (int16_t)Config::getSetting("leds-typ").toInt();
            auto pixelsSpeed = (int16_t)Config::getSetting("leds-spd").toInt();
            pixels = new Adafruit_NeoPixel(count, pin, pixelsType + pixelsSpeed);
            // turn off
            for (int i = 0; i < count; i++) {
                pixels->setPixelColor(i, 0, 0, 0);
            }
        }

        // Setup Status LED as master channel
        mainModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Main");
        mainModule->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(color);
            fx_main(color);
        });
        homeGenie->addAPIHandler(mainModule);

        // Setup example input "processor" module
        // changing color of this module will affect
        // all LED pixels with a color cycle effect
        rainbowModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "F1", "Rainbow");
        auto soundLightFeature = new ModuleParameter("Widget.Preference.AudioLight", "true");
        rainbowModule->module->properties.add(soundLightFeature);
        rainbowModule->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(color);
            fx_rainbow(color);
        });
        homeGenie->addAPIHandler(rainbowModule);

    }

    homeGenie->begin();

    // TODO: implement color/status recall on start
    // TODO: implement color/status recall on start
    // TODO: implement color/status recall on start

    if (statusLED != nullptr) {
        statusLED->begin();
    }
    if (pixels != nullptr) {
        pixels->begin();
    }

    refresh();
}

void loop()
{
    homeGenie->loop();

    if (isConfigured) {
        // refresh background/strobe layer
        if (currentColor.isAnimating()) {
            refresh();
        }

        // 40 fps animation FX layer
        if (millis() - lastRefreshTs > 25) {

            // FX - rainbow animation
            if (rainbowModule->isOn()) {
                fx_rainbow(currentColor);
                refresh();
            }
            // TODO: add more FX modules

            lastRefreshTs = millis();
        }
    }
}
