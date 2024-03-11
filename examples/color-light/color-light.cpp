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

#include "api/ColorLight.h"

using namespace Service;

HomeGenie* homeGenie;

Adafruit_NeoPixel pixels(1, CONFIG_StatusLedNeoPixelPin, NEO_GRB + NEO_KHZ800);

void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(50, 50, 0));
    } else {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    pixels.show();
}

void setup() {
    pixels.begin();

    homeGenie = HomeGenie::getInstance();

    if (!Config::isDeviceConfigured()) {

        // Custom status led (builtin NeoPixel RGB on pin 10)
        Config::statusLedCallback(&statusLedCallback);

    } else {

        auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Demo Light");
        colorLight->onSetColor([](float r, float g, float b) {
            pixels.setPixelColor(0, r, g, b);
            pixels.show();
        });
        homeGenie->addAPIHandler(colorLight);

        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start
        // TODO: implement color/status recall on start

    }

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}
