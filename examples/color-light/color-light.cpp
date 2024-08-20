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
uint16_t ledsCount = 0; int16_t ledsPin = -1;
int16_t pixelsType, pixelsSpeed;
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

ModuleParameter* mpLedCount;
ModuleParameter* mpFxStyle;
ModuleParameter* mpFxStrobe;

unsigned long strobeFxTickMs = 0;
unsigned int strobeFxDurationMs = 20;
unsigned int strobeFxIntervalMs = 80;  // limit strobe to 10Hz  (20+80 -> 100ms interval)
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

void renderPixels() {
    if (pixels != nullptr) {
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i,
                                  animatedColors[i]->getRed(),
                                  animatedColors[i]->getGreen(),
                                  animatedColors[i]->getBlue());
        }
    }
    if (statusLED != nullptr) {
        statusLED->setPixelColor(0, currentColor.getRed(), currentColor.getGreen(), currentColor.getBlue());
    }
}

void createPixels()
{
    if (ledsPin >= 0) {
        pixels = new Adafruit_NeoPixel(ledsCount, ledsPin, pixelsType + pixelsSpeed);
        // turn off
        //pixels->clear();
        //*
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }
        //*/
        pixels->begin();
    }
}
void disposePixels() {

    // turn off all pixels
    if (pixels != nullptr) {
        //pixels->clear();
        //*
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }
        refresh();
        //*/
        delete pixels;
        delay(1000);
    }

}

void setup() {
    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->name = "Smart light";

    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.LED.count",
                                "number:LED.count:1:1920:1:led_count"));
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Rainbow",
                                "select:FX.Style:light_style:solid|rainbow|rainbow_2|white_stripes|white_stripes_2|kaleidoscope"));
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Strobe",
                                "select:FX.Strobe:strobe_effect:off|slow|medium|fast"));

    mpLedCount = new ModuleParameter("LED.count", "0");
    miniModule->properties.add(mpLedCount);
    mpFxStyle = new ModuleParameter("FX.Style", currentStyle);
    miniModule->properties.add(mpFxStyle);
    mpFxStrobe = new ModuleParameter("FX.Strobe", "off");
    miniModule->properties.add(mpFxStrobe);

    // Get status LED config
    int statusLedPin = Config::getSetting("stld-pin", "-1").toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ", "6").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
        statusLED->begin();
    }

    isConfigured = Config::isDeviceConfigured();
    if (!isConfigured) {

        // Custom status led (builtin NeoPixel RGB on pin 10)
        if (statusLED != nullptr) {
            Config::statusLedCallback(&statusLedCallback);
        }

        Config::setOnWiFiConfiguredCallback([]{
            // Device successfully configured
            Serial.println("Restarting now.");
            delay(2000);
            ESP.restart();
            // TODO: re-arrange code so that it won't require a restart
        });

    } else {

        // Get LED strip config
        ledsCount = abs(Config::getSetting("leds-cnt").toInt());
        ledsPin = (int16_t)Config::getSetting("leds-pin").toInt();
        pixelsType = (int16_t)Config::getSetting("leds-typ").toInt();
        pixelsSpeed = (int16_t)Config::getSetting("leds-spd").toInt();
        createPixels();

        mpLedCount->value = String(ledsCount);

        // Setup main LEDs control module
        mainModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Main");
        mainModule->module->properties.add(
                new ModuleParameter("Widget.Preference.AudioLight", "true"));
        mainModule->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(pixels, color);
        });
        homeGenie->addAPIHandler(mainModule);

        // TODO: add 2 "GpioButton" modules for handling manual control (on/off, level, switch fx, ...)

        // Initialize FX buffer
        fx_init(ledsCount, currentColor);

        // TODO: implement color/status recall on start

    }

    homeGenie->begin();

    refresh();
}

void loop()
{
    homeGenie->loop();

    if (isConfigured) {

        if (millis() - lastRefreshTs > refreshMs)
        {
            // Update current rendering style if changed
            if (currentStyle != mpFxStyle->value) {
                currentStyle = mpFxStyle->value;
            }

            // enable / disable strobe light
            if (strobeFxTickMs > 0 && mpFxStrobe->value == "off") {
                strobeFxTickMs = 0;
            } else if (strobeFxTickMs == 0 && mpFxStrobe->value != "off") {
                strobeFxTickMs = millis();
            }


            if (!strobeOff && strobeFxTickMs > 0 && millis() - strobeFxTickMs > ((strobeFxDurationMs + strobeFxIntervalMs) * (mpFxStrobe->value == "slow" ? 3 : mpFxStrobe->value == "medium" ? 2 : 1))) {

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
                    fx_rainbow(pixels, currentColor, 1);
                } else if (currentStyle == "rainbow_2") {
                    fx_rainbow(pixels, currentColor, 2);
                } else if (currentStyle == "kaleidoscope") {
                    fx_kaleidoscope(pixels, currentColor);
                } else if (currentStyle == "white_stripes") {
                    fx_white_stripes(pixels, currentColor);
                } else if (currentStyle == "white_stripes_2") {
                    fx_white_stripes(pixels, currentColor, true);
                }

            }

            renderPixels();
            // show pixels
            refresh();

            lastRefreshTs = millis();
            if (strobeOff) {
                strobeFxTickMs = lastRefreshTs;
            }
        }

        // check if number of pixels was changed
        if (ledsCount != mpLedCount->value.toInt()) {

            disposePixels();

            ledsCount = mpLedCount->value.toInt();
            Config::saveSetting("leds-cnt", mpLedCount->value);

            // re-create pixels with the new length
            createPixels();

        }

    }
}
