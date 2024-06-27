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

void show() {
    if (changed && millis() - lastRefreshTs > 25) { // force 40 fps max
        if (statusLED != nullptr) {
            statusLED->show();
        }
        if (pixels != nullptr) {
            pixels->show();
        }
        lastRefreshTs = millis();
        changed = false;
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
    }

    if (!Config::isDeviceConfigured()) {

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
        }

        // Setup Status LED as master channel
        auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Main");
        colorLight->onSetColor([](LightColor color) {
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

            changed = true;
            show();
        });
        homeGenie->addAPIHandler(colorLight);

        // Setup example input "processor" module
        // changing color of this module will affect
        // all LED pixels with a color cycle effect
        auto fxModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "F1", "Rainbow");
        auto soundLightFeature = new ModuleParameter("Widget.Preference.AudioLight", "true");
        fxModule->module->properties.add(soundLightFeature);
        fxModule->onSetColor([](LightColor color) {

            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, color.getRed(), color.getGreen(), color.getBlue());
            }

            if (pixels != nullptr) {
                float s = color.getSaturation();
                if (s > 1) {
                    s = 1;
                }
                float h = color.getHue();
                float hueStep = 1.0f / (float) count;
                float v = color.getValue();
                for (int i = 0; i < count; i++) {
                    h += hueStep;
                    auto rgb = Utility::hsv2rgb(h, s, v);
                    pixels->setPixelColor(i, rgb.r, rgb.g, rgb.b);
                }
            }

            changed = true;
            show();

        });
        homeGenie->addAPIHandler(fxModule);

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
}

void loop()
{
    homeGenie->loop();
    show();
}
