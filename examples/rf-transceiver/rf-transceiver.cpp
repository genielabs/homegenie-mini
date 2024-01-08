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
#include "api/RCSwitchHandler.h"

using namespace Service;

HomeGenie* homeGenie;

void setup() {

    homeGenie = HomeGenie::getInstance();
    auto miniModule = homeGenie->getDefaultModule();


    // RCSwitch RF Transmitter
    auto rcsTransmitterConfig = new RCS::RFTransmitterConfig(CONFIG_RCSwitchTransmitterPin);
    auto rcsTransmitter = new RCS::RFTransmitter(rcsTransmitterConfig);
    homeGenie->addAPIHandler(new RCSwitchHandler(rcsTransmitter));

    // TODO:    homeGenie->addIOHandler(new RCS::RFReceiver());
    // TODO:    auto propRawData = new ModuleParameter(IOEventPaths::Receiver_RawData);
    // TODO:    miniModule->properties.add(propRawData);


    homeGenie->begin();

}

void loop()
{
    homeGenie->loop();
}
