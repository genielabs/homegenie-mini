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

#include <Adafruit_NeoPixel.h>

#include "configuration.h"
#include "color-fx.h"

using namespace Service;
using namespace Service::API::devices;

HomeGenie* homeGenie;
bool isConfigured = false;

// Optional RGB Status LED
Adafruit_NeoPixel* statusLED = nullptr;

// LED strip/array
int count = 0; int pin = -1;
Adafruit_NeoPixel* pixels = nullptr;

// LED Blink callback when statusLED is configured
void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(1, 1, 0));
    } else {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    statusLED->show();
}

unsigned int refreshMs = 10;
unsigned long lastRefreshTs = 0;

LightColor currentColor;
ColorLight* mainModule;

ModuleParameter* fxStyle;
ModuleParameter* fxStrobe;

bool playFxStrobe;
String currentStyle = "solid";

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
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->name = "Smart light";
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Rainbow",
                                "select:FX.Style:light_style:solid|rainbow|white_stripes|kaleidoscope"));
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Strobe",
                                "select:FX.Strobe:strobe_effect:off|slow|medium|fast"));

    fxStyle = new ModuleParameter("FX.Style", "solid");
    miniModule->properties.add(fxStyle);
    fxStrobe = new ModuleParameter("FX.Strobe", "false");
    miniModule->properties.add(fxStrobe);

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

        Config::setOnWiFiConfiguredCallback([]{
            Serial.println("Restarting now.");
            delay(2000);
            ESP.restart();
        });

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

        // Setup main LEDs control module
        mainModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Main");
        mainModule->module->properties.add(
                new ModuleParameter("Widget.Preference.AudioLight", "true"));
        mainModule->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(pixels, color);
            fx_solid(pixels, color);
        });
        homeGenie->addAPIHandler(mainModule);

        // Allocate "animated" colors
        animatedColors = new AnimatedColor[count];
        for (int i = 0; i < count; i++) {
            animatedColors[i] = new LightColor();
            animatedColors[i]->setColor(currentColor.getHue(), currentColor.getSaturation(), currentColor.getValue(), 0);
        }

    }

    homeGenie->begin();

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
        if (currentColor.isAnimating() && currentStyle == "solid") {
            refresh();
        }

        // 40 fps animation FX layer
        if (millis() - lastRefreshTs > refreshMs) {

            // Check if current rendering style changed and update fx switches
            if (currentStyle != fxStyle->value) {
                currentStyle = fxStyle->value;
                if (currentStyle == "solid") {
                    fx_solid(pixels, currentColor);
                    refresh();
                }
            }

            if (playFxStrobe && fxStrobe->value == "off") {
                playFxStrobe = false;
                fx_solid(pixels, currentColor);
                refresh();
            } else if (!playFxStrobe && (fxStrobe->value == "slow" || fxStrobe->value == "medium" || fxStrobe->value == "fast")) {
                playFxStrobe = true;
            }

            if (currentStyle != "solid") {
                // overlay selected effect
                if (currentStyle == "rainbow") {
                    fx_rainbow(pixels, currentColor);
                } else if (currentStyle == "kaleidoscope") {
                    fx_kaleidoscope(pixels, currentColor);
                } else if (currentStyle == "white_stripes") {
                    fx_white_stripes(pixels, currentColor);
                }
                refresh();
            }

            if (playFxStrobe) {
                float speed = 2;
                if (fxStrobe->value == "medium") {
                    speed = 4;
                }
                if (fxStrobe->value == "slow") {
                    speed = 6;
                }
                // TODO: rewrite without using delays
                delay(refreshMs * speed);
                // invert solid color
                auto c = LightColor();
                c.setColor(0, 0, 1, 0);
                fx_solid(pixels, c);
                refresh();
                delay(refreshMs);
                // restore solid color
                fx_solid(pixels, currentColor);
                refresh();
                delay(refreshMs);
            }

            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, currentColor.getRed(), currentColor.getGreen(), currentColor.getBlue());
            }

            lastRefreshTs = millis();
        }
    }
}
