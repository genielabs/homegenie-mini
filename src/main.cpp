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
#include <scripting/ProgramEngine.h>

#include "service/HomeGenie.h"
#include <TaskManager.h>

#define HOMEGENIE_MINI_VERSION "1.0"

using namespace IO;
using namespace Net;
using namespace Service;

HomeGenie homeGenie;

/// This gets called just before the main application loop()
void setup() {

    // Logger initialization
    Logger::begin(LOG_LEVEL_TRACE);

    // Welcome message
    Logger::info("HomeGenie-Mini %s", HOMEGENIE_MINI_VERSION);
    Logger::info("Booting...");

    Logger::info("+ Starting HomeGenie service");
    homeGenie.begin();

    Logger::info("READY.");

}

/// Main application loop
void loop()
{
    // TODO: sort of Load index could be obtained by measuring time elapsed for `TaskManager::loop()` method
    TaskManager::loop();
}

//////////////////////////////////////////
