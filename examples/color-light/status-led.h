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

#include <Adafruit_NeoPixel.h>

// Optional RGB Status LED
Adafruit_NeoPixel* statusLED = nullptr;
bool _statusLedWifiConnected = true;

// Status LED Blink callback used if statusLED is enabled
void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(1, 1, 0));
    } else {
        statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    statusLED->show();
}

void statusLedLoop() {
    if (statusLED != nullptr && WiFi.isConnected() != _statusLedWifiConnected) {
        _statusLedWifiConnected = WiFi.isConnected();
        if (_statusLedWifiConnected) {
            Config::statusLedCallback(nullptr);
        } else {
            Config::statusLedCallback(&statusLedCallback);
        }
    }
}
