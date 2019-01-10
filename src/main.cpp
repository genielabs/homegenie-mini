/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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
 * - 2019-10-01 Initial release
 *
 */

#include <Arduino.h>
#include <net/NetManager.h>
#include <io/Logger.h>

#include "service/HomeGenie.h"

#define HOMEGENIE_MINI_VERSION "1.0"

using namespace IO;
using namespace Net;
using namespace Service;

NetManager netManager;
HomeGenie homeGenie;

/// This gets called just before the main application loop()
void setup() {
    // Logger initialization
    Logger::begin();
    // Welcome message
    Logger::info("HomeGenie Mini V%s", HOMEGENIE_MINI_VERSION);
    Logger::info("Booting...");

    // WI-FI will not boot without this delay!!!
    delay(2000);

    Logger::info("+ Starting NetManager");
    netManager.begin();

    Logger::info("+ Starting HomeGenie service");
    homeGenie.begin();
}

/// Main application loop
void loop()
{
    Logger::loop();
    homeGenie.loop();
}

//////////////////////////////////////////
