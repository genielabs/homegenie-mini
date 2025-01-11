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
#include "../color-light/status-led.h"
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
    // Custom status led (builtin NeoPixel RGB LED)
    auto colorLight = statusLedSetup();
    colorLight->onSetColor([](LightColor c) {
        statusLED->setPixelColor(0, c.getRed(), c.getGreen(), c.getBlue());
        statusLED->show();
    });
    homeGenie->addAPIHandler(colorLight);
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
    statusLedLoop();
#endif
}
