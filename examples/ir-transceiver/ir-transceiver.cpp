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
 */

#include <HomeGenie.h>

#include "configuration.h"

#include "io/IRReceiver.h"
#include "io/IRTransmitter.h"
#include "api/IRTransceiverHandler.h"

using namespace Service;

HomeGenie* homeGenie;

#ifdef ESP32_C3
#include <Adafruit_NeoPixel.h>
// Custom status led (builtin NeoPixel RGB on pin 10)
Adafruit_NeoPixel pixels(1, 10, NEO_GRB + NEO_KHZ800);
void statusLedCallback(bool isLedOn) {
    if (isLedOn) {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(50, 50, 0));
    } else {
        pixels.setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
    }
    pixels.show();
}
#endif

void setup() {
#ifdef ESP32_C3
    // Custom status led (builtin NeoPixel RGB on pin 10)
    Config::statusLedCallback(&statusLedCallback);
    pixels.begin();
#endif

    homeGenie = HomeGenie::getInstance();

    auto receiverConfiguration = new IR::IRReceiverConfig(CONFIG_IRReceiverPin);
    auto receiver = new IR::IRReceiver(receiverConfiguration);
    homeGenie->addIOHandler(receiver);

    auto transmitterConfig = new IR::IRTransmitterConfig(CONFIG_IRTransmitterPin);
    auto transmitter = new IR::IRTransmitter(transmitterConfig);
    homeGenie->addAPIHandler(new IRTransceiverHandler(transmitter, receiver));

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}