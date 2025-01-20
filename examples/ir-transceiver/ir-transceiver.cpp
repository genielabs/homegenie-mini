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
 */

#include <HomeGenie.h>

#include "configuration.h"

#include "io/IRReceiver.h"
#include "io/IRTransmitter.h"
#include "api/IRTransceiverHandler.h"

#ifdef BOARD_HAS_RGB_LED
#include "../color-light/StatusLed.h"
StatusLed statusLed;
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
    statusLed.setup();
#endif

    auto apiHandler = new IRTransceiverHandler();
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

    homeGenie->addAPIHandler(apiHandler);

    homeGenie->begin();
}


void loop()
{
    homeGenie->loop();
}
