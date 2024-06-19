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

void setup() {
    homeGenie = HomeGenie::getInstance();

    // Get status LED config
    int statusLedPin = Config::getSetting("stld-pin", "0").toInt();
    if (statusLedPin > 0) {
        int statusLedType = Config::getSetting("stld-typ", "0").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
    }

    if (!Config::isDeviceConfigured()) {

        // Custom status led (builtin NeoPixel RGB on pin 10)
        if (statusLED != nullptr) {
            Config::statusLedCallback(&statusLedCallback);
        }

    } else {

        // Get LED strip config
        count = Config::getSetting("leds-cnt", "0").toInt();  // 90 = 3mt // 30 LEDs per meter (3 mt. strip)
        pin = (int16_t)Config::getSetting("leds-pin", "-1").toInt();
        if (count > 0 && pin != -1) {
            auto pixelsType = (int16_t)Config::getSetting("leds-typ", "-1").toInt();
            auto pixelsSpeed = (int16_t)Config::getSetting("leds-spd", "0").toInt();
            pixels = new Adafruit_NeoPixel(count, pin, pixelsType + pixelsSpeed);
        }

        // Setup Status LED as master channel
        auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Demo Light");
        colorLight->onSetColor([](float r, float g, float b) {
            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, r, g, b);
            }

            for (int i = 0; i < count; i++) {
                pixels->setPixelColor(i, r, g, b);
            }

            changed = true;
            if (millis() - lastRefreshTs > 50) { // force 20fps max
                if (statusLED != nullptr) {
                    statusLED->show();
                }
                if (pixels != nullptr) {
                    pixels->show();
                }
                lastRefreshTs = millis();
                changed = false;
            }
        });
        homeGenie->addAPIHandler(colorLight);

        // Setup LED strip/array
        if (pixels != nullptr) {
            for (int i = 0; i < count; i++) {
                auto address = String("L") + String(i + 1);
                auto cl = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, address.c_str(), "Demo Light");
                cl->onSetColor([i](float r, float g, float b) {
                    pixels->setPixelColor(i, r, g, b);
                    changed = true;
                    if (millis() - lastRefreshTs > 50) { // force 20fps max
                        pixels->show();
                        lastRefreshTs = millis();
                        changed = false;
                    }
                });
                homeGenie->addAPIHandler(cl);
            }
        }
        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start

    }

    if (statusLED != nullptr) {
        statusLED->begin();
    }
    if (pixels != nullptr) {
        pixels->begin();
    }
    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();

    if (changed) { // trailing fx
        changed = false;
        if (statusLED != nullptr) {
            statusLED->show();
        }
        if (pixels != nullptr) {
            pixels->show();
        }
        lastRefreshTs = millis();
    }
}
