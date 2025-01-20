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

#include "configuration.h"
#include "io/RFTransmitter.h"
#include "io/RFReceiver.h"
#include "api/RCSwitchHandler.h"

#ifdef BOARD_HAS_RGB_LED
#include "../color-light/StatusLed.h"
StatusLed statusLed;
#endif

using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Firefly RF";

    homeGenie = HomeGenie::getInstance();

#ifdef BOARD_HAS_RGB_LED
    statusLed.setup();
#endif

    auto apiHandler = new RCSwitchHandler();
#ifdef BOARD_HAS_RGB_LED
    apiHandler->setOnDataReady([](const char* c) {
        statusLed.signalActivity(0, 255, 255);
    });
#endif

    // RCSwitch RF Receiver
    uint8_t rfReceiverPin = Config::getSetting("rfrc-pin", CONFIG_RCSwitchReceiverPin).toInt();
    if (rfReceiverPin > 0) {
        auto receiverConfiguration = new RCS::RFReceiverConfig(rfReceiverPin);
        auto rcsReceiver = new RCS::RFReceiver(receiverConfiguration);
        apiHandler->setReceiver(rcsReceiver);
        homeGenie->addIOHandler(rcsReceiver);
    }
    // RCSwitch RF Transmitter
    uint8_t rfTransmitterPin = Config::getSetting("rftr-pin", CONFIG_RCSwitchTransmitterPin).toInt();
    if (rfTransmitterPin > 0) {
        auto rcsTransmitterConfig = new RCS::RFTransmitterConfig(rfTransmitterPin);
        auto rcsTransmitter = new RCS::RFTransmitter(rcsTransmitterConfig);
        apiHandler->setTransmitter(rcsTransmitter);
    }

    homeGenie->addAPIHandler(apiHandler);

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}
