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
 * Releases:
 * - 2019-01-10 v1.0: initial release.
 *
 */

#include <HomeGenie.h>
#include <service/api/devices/ColorLight.h>

#include "../color-light/status-led.h"
#include "CommonSensors.h"

using namespace Service;
using namespace Service::API::devices;

HomeGenie* homeGenie;
Module* miniModule;

void setup() {
    // Default name shown in SMNP/UPnP advertising
    Config::system.friendlyName = "Smart Sensor";

    homeGenie = HomeGenie::getInstance();

    miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty("Widget.Implements.Scheduling", "1");
    miniModule->setProperty("Widget.Implements.Scheduling.ModuleEvents", "1");

    // Get status LED config
    auto pin = Config::getSetting("stld-pin");
    int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ", "82").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
        statusLED->begin();
    }

    // Custom status led (builtin NeoPixel RGB LED)
    if (statusLED != nullptr) {

        // Setup main LEDs control module
        auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Status LED");
        colorLight->module->setProperty("Widget.Implements.Scheduling", "1");
        colorLight->module->setProperty("Widget.Implements.Scheduling.ModuleEvents", "1");
        colorLight->module->setProperty("Widget.Preference.AudioLight", "true");
        colorLight->onSetColor([](LightColor c) {
            statusLED->setPixelColor(0, c.getRed(), c.getGreen(), c.getBlue());
            statusLED->show();
        });
        homeGenie->addAPIHandler(colorLight);

    }

    includeCommonSensors(homeGenie, miniModule);

    homeGenie->begin();
}

void loop()
{

    homeGenie->loop();

    // Custom status led (builtin NeoPixel RGB LED)
    statusLedLoop();

}
