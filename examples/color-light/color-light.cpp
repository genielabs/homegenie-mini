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

unsigned long strobeFxTickMs = 0;
unsigned int strobeFxDurationMs = 25;
unsigned int strobeFxIntervalMs = 75;  // limit strobe to 10Hz  (25+75 -> 100ms interval)
bool strobeOff = true;

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

    fxStyle = new ModuleParameter("FX.Style", currentStyle);
    miniModule->properties.add(fxStyle);
    fxStrobe = new ModuleParameter("FX.Strobe", "off");
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
        });
        homeGenie->addAPIHandler(mainModule);

        // Initialize FX buffer
        fx_init(count, currentColor);
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

        if (millis() - lastRefreshTs > refreshMs)
        {
            // Update current rendering style if changed
            if (currentStyle != fxStyle->value) {
                currentStyle = fxStyle->value;
            }

            // enable / disable strobe light
            if (strobeFxTickMs > 0 && fxStrobe->value == "off") {
                strobeFxTickMs = 0;
            } else if (strobeFxTickMs == 0 && fxStrobe->value != "off") {
                strobeFxTickMs = millis();
            }


            if (!strobeOff && strobeFxTickMs > 0 && millis() - strobeFxTickMs > ((strobeFxDurationMs + strobeFxIntervalMs) * (fxStrobe->value == "slow" ? 3 : fxStrobe->value == "medium" ? 2 : 1))) {

                // strobe off
                strobeOff = true;

            } else if (strobeFxTickMs > 0 && (millis() - strobeFxTickMs <= strobeFxDurationMs || strobeOff)) {

                // show strobe light for `strobeFxDurationMs` milliseconds (25)
                auto c = LightColor();
                c.setColor(0, 0, 1, 0);
                fx_solid(pixels, c, 0);
                strobeOff = false;

            } else {

                // apply selected light style
                if (currentStyle == "solid") {
                    fx_solid(pixels, currentColor, strobeFxTickMs > 0 ? 0 : 200);
                } else if (currentStyle == "rainbow") {
                    fx_rainbow(pixels, currentColor);
                } else if (currentStyle == "kaleidoscope") {
                    fx_kaleidoscope(pixels, currentColor);
                } else if (currentStyle == "white_stripes") {
                    fx_white_stripes(pixels, currentColor);
                }

            }


            // render pixels
            if (pixels != nullptr) {
                for (int i = 0; i < pixels->numPixels(); i++) {
                    pixels->setPixelColor(i,
                                          animatedColors[i]->getRed(),
                                          animatedColors[i]->getGreen(),
                                          animatedColors[i]->getBlue());
                }
            }
            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, currentColor.getRed(), currentColor.getGreen(), currentColor.getBlue());
            }

            // show pixels
            refresh();

            lastRefreshTs = millis();
            if (strobeOff) {
                strobeFxTickMs = lastRefreshTs;
            }
        }

    }
}
