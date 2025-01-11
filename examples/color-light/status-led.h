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
 */

#ifndef HOMEGENIE_MINI_STATUS_LED_H_
#define HOMEGENIE_MINI_STATUS_LED_H_

#include <Config.h>
#include <service/api/devices/ColorLight.h>

#include <Adafruit_NeoPixel.h>

#define COLOR_LIGHT_ADDRESS "C1"

using namespace Service::API::devices;

// Optional RGB Status LED
Adafruit_NeoPixel* statusLED = nullptr;
bool _statusLedWifiConnected = true;

// Blinks only first 20 seconds right after boot
unsigned long helloWorldDuration = 20000;
bool helloWorldActive = true;

// Status LED Blink callback used if statusLED is enabled
void statusLedCallback(bool isLedOn) {
    if (statusLED == nullptr) return;
    if (isLedOn) {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(1, 1, 0));
    } else {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    statusLED->show();
}

ColorLight* statusLedSetup() {

#ifdef BOARD_HAS_RGB_LED
    // Get status LED config
    auto pin = Config::getSetting("stld-pin");
    int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ", "82").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
        statusLED->begin();
        Config::statusLedCallback(&statusLedCallback);
    }
#endif
    // Setup master LED control module
    auto colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Color Light");
    colorLight->module->setProperty("Widget.Implements.Scheduling", "1");
    colorLight->module->setProperty("Widget.Implements.Scheduling.ModuleEvents", "1");
    colorLight->module->setProperty("Widget.Preference.AudioLight", "true");

    return colorLight;
}

void statusLedLoop() {
    if (statusLED != nullptr) {
        if (helloWorldActive && millis() > helloWorldDuration && Config::isDeviceConfigured()) {
            // stops blinking if "helloWorldDuration" is elapsed
            helloWorldActive = false;
            Config::statusLedCallback(nullptr);
        } else if (WiFi.isConnected() != _statusLedWifiConnected) {
            _statusLedWifiConnected = WiFi.isConnected();
            if (_statusLedWifiConnected) {
                // turn off status LED blinking if connected
                Config::statusLedCallback(nullptr);
            } else {
                // blinks status LED rapidly if not connected
                Config::statusLedCallback(&statusLedCallback);
            }
        }
    }
}

#endif