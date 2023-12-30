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
 * - 2023-12-08 v1,1: added BLE support; targeting ESP32 by default.
 *
 */


#include <HomeGenie.h>

#include "io/DS18B20.h"
#include "io/LightSensor.h"

using namespace IO::Env;
using namespace Service;

HomeGenie* homeGenie;

void setup() {

    homeGenie = HomeGenie::getInstance();

    auto miniModule = homeGenie->getDefaultModule();

    // Temperature sensor
    homeGenie->addIOHandler(new DS18B20());
    auto temperature = new ModuleParameter(IOEventPaths::Sensor_Temperature);
    miniModule->properties.add(temperature);

    // Light sensor
    homeGenie->addIOHandler(new LightSensor());
    auto luminance = new ModuleParameter(IOEventPaths::Sensor_Luminance);
    miniModule->properties.add(luminance);

    homeGenie->begin();

}

void loop()
{

    homeGenie->loop();

}
