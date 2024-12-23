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

using namespace IO;
using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SMNP/UPnP advertising
    Config::system.friendlyName = "Firefly X10";

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();

    // X10 RF RFTransmitter
    auto x10TransmitterConfig = new X10::RFTransmitterConfig(CONFIG_X10RFTransmitterPin);
    auto x10Transmitter = new X10::RFTransmitter(x10TransmitterConfig);
    // X10 RF RFReceiver
    auto x10ReceiverConfig = new X10::RFReceiverConfig(CONFIG_X10RFReceiverPin);
    auto x10Receiver = new X10::RFReceiver(x10ReceiverConfig);

    auto apiHandler = new X10Handler(x10Transmitter, x10Receiver);
    homeGenie->addAPIHandler(apiHandler);

    auto rfModule = apiHandler->getModule(IO::IOEventDomains::HomeAutomation_X10, CONFIG_X10RF_MODULE_ADDRESS);
    x10Receiver->setModule(rfModule);
    homeGenie->addIOHandler(x10Receiver);

    homeGenie->begin();
}

void loop()
{

    homeGenie->loop();

}
