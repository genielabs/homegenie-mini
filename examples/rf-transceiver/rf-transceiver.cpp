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

using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SMNP/UPnP advertising
    Config::system.friendlyName = "Firefly RF";

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();

    // RCSwitch RF Transmitter
    auto rcsTransmitterConfig = new RCS::RFTransmitterConfig(CONFIG_RCSwitchTransmitterPin);
    auto rcsTransmitter = new RCS::RFTransmitter(rcsTransmitterConfig);

    auto apiHandler = new RCSwitchHandler(rcsTransmitter);
    homeGenie->addAPIHandler(apiHandler);

    // RCSwitch RF Receiver
    auto rfModule = apiHandler->getModule(IO::IOEventDomains::HomeAutomation_RemoteControl, CONFIG_RCSwitchRF_MODULE_ADDRESS);
    auto receiverConfiguration = new RCS::RFReceiverConfig(CONFIG_RCSwitchReceiverPin);
    auto receiver = new RCS::RFReceiver(receiverConfiguration);
    receiver->setModule(rfModule);
    homeGenie->addIOHandler(receiver);

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}
