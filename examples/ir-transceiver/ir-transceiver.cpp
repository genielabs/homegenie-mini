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

#ifdef BOARD_HAS_RGB_LED
#include <service/api/devices/ColorLight.h>
#include "../color-light/status-led.h"
using namespace Service::API::devices;
unsigned long helloWorldDuration = 10000;
bool helloWorldActive = true;
#endif

using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Firefly IR";

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty("Widget.Implements.Scheduling", "1");

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
#endif

    auto apiHandler = new IRTransceiverHandler();
    homeGenie->addAPIHandler(apiHandler);
    // IR receiver pin
    uint8_t irReceiverPin = Config::getSetting("irrc-pin", String(CONFIG_IRReceiverPin).c_str()).toInt();
    if (irReceiverPin > 0) {
        auto receiverConfiguration = new IR::IRReceiverConfig(irReceiverPin);
        auto receiver = new IR::IRReceiver(receiverConfiguration);
        apiHandler->setReceiver(receiver);
        homeGenie->addIOHandler(receiver);
    }
    // IR transmitter pin
    uint8_t irTransmitterPin = Config::getSetting("irtr-pin", String(CONFIG_IRTransmitterPin).c_str()).toInt();
    if (irTransmitterPin > 0) {
        auto transmitterConfig = new IR::IRTransmitterConfig(irTransmitterPin);
        auto transmitter = new IR::IRTransmitter(transmitterConfig);
        apiHandler->setTransmitter(transmitter);
    }

    homeGenie->begin();
}


void loop()
{
    homeGenie->loop();
#ifdef BOARD_HAS_RGB_LED
    if (statusLED != nullptr) {
        if (helloWorldActive && millis() > helloWorldDuration && Config::isDeviceConfigured()) {
            helloWorldActive = false;
            Config::statusLedCallback(nullptr);
        }
    }
#endif
}
