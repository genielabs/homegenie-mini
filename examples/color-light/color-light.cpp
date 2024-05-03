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

Adafruit_NeoPixel statusLED(1, CONFIG_StatusLedNeoPixelPin, NEO_GRB + NEO_KHZ800);

#ifdef LED_ARRAY_COUNT
int num = LED_ARRAY_COUNT;  // 90 = 3mt // 30 LEDs per meter (3 mt. strip)
int pin = LED_ARRAY_PIN;
Adafruit_NeoPixel pixels(num, pin, NEO_RGB + NEO_KHZ800);
#endif

bool changed = false;
unsigned long lastRefreshTs = 0;

void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        statusLED.setPixelColor(0, Adafruit_NeoPixel::Color(50, 50, 0));
    } else {
        statusLED.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    statusLED.show();
}

void setup() {
    homeGenie = HomeGenie::getInstance();

    if (!Config::isDeviceConfigured()) {

        // Custom status led (builtin NeoPixel RGB on pin 10)
        Config::statusLedCallback(&statusLedCallback);

    } else {

        auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Demo Light");
        colorLight->onSetColor([](float r, float g, float b) {
            statusLED.setPixelColor(0, r, g, b);
#ifdef LED_ARRAY_COUNT
            for (int i = 0; i < num; i++) {
                pixels.setPixelColor(i, r, g, b);
            }
#endif
            changed = true;
            if (millis() - lastRefreshTs > 50) { // force 20fps max
                statusLED.show();
#ifdef LED_ARRAY_COUNT
                pixels.show();
#endif
                lastRefreshTs = millis();
                changed = false;
            }
        });
        homeGenie->addAPIHandler(colorLight);

#ifdef LED_ARRAY_COUNT
        for (int i = 0; i < num; i++) {
            auto address = String("L") + String(i + 1);
            auto cl = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, address.c_str(), "Demo Light");
            cl->onSetColor([i](float r, float g, float b) {
                pixels.setPixelColor(i, r, g, b);
                changed = true;
                if (millis() - lastRefreshTs > 50) { // force 20fps max
                    pixels.show();
                    lastRefreshTs = millis();
                    changed = false;
                }
            });
            homeGenie->addAPIHandler(cl);
        }
#endif

        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start

    }

    statusLED.begin();
    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
    if (changed) { // trailing fx
        changed = false;
        statusLED.show();
#ifdef LED_ARRAY_COUNT
        pixels.show();
#endif
        lastRefreshTs = millis();
    }
}
