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

#include "api/X10Handler.h"
#include "io/RFReceiver.h"
#include "io/RFTransmitter.h"

#ifdef BOARD_HAS_RGB_LED
#include <service/api/devices/ColorLight.h>
#include "../color-light/status-led.h"
using namespace Service::API::devices;
unsigned long helloWorldDuration = 10000;
bool helloWorldActive = true;
#endif

using namespace IO;
using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Firefly X10";

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();

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
        auto colorLight = new ColorLight(IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Status LED");
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

    auto apiHandler = new X10Handler();
    auto rfModule = apiHandler->getModule(IOEventDomains::HomeAutomation_X10, CONFIG_X10RF_MODULE_ADDRESS);

    // X10 RF RFReceiver
    uint8_t rfReceiverPin = Config::getSetting("rfrc-pin", String(CONFIG_X10RFReceiverPin).c_str()).toInt();
    if (rfReceiverPin > 0) {
        auto x10ReceiverConfig = new X10::RFReceiverConfig(rfReceiverPin);
        auto x10Receiver = new X10::RFReceiver(x10ReceiverConfig);
        x10Receiver->setModule(rfModule);
        apiHandler->setReceiver(x10Receiver);
        homeGenie->addIOHandler(x10Receiver);
    }

    // X10 RF RFTransmitter
    uint8_t rfTransmitterPin = Config::getSetting("rftr-pin", String(CONFIG_X10RFTransmitterPin).c_str()).toInt();
    if (rfTransmitterPin > 0) {
        auto x10TransmitterConfig = new X10::RFTransmitterConfig(rfTransmitterPin);
        auto x10Transmitter = new X10::RFTransmitter(x10TransmitterConfig);
        apiHandler->setTransmitter(x10Transmitter);
    }

    homeGenie->addAPIHandler(apiHandler);

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
