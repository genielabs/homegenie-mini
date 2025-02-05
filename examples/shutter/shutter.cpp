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

#include "configuration.h"

#include <HomeGenie.h>

#include "api/ShutterHandler.h"

#ifdef BOARD_HAS_RGB_LED
#include "../color-light/StatusLed.h"
StatusLed statusLed;
#endif

using namespace IO;
using namespace Service;

HomeGenie* homeGenie;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Smart Shutter";

    homeGenie = HomeGenie::getInstance();

#ifdef BOARD_HAS_RGB_LED
    // Custom status led (builtin NeoPixel RGB LED)
    statusLed.setup();
#endif

    auto shutterHandler = new ShutterHandler();
    homeGenie->addAPIHandler(shutterHandler);

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}
