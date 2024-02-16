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

#include <ui/Dashboard.h>
#include <ui/drivers/RoundDisplay.h>

//#include <ui/activities/examples//GaugeExampleActivity.h>
#include <ui/activities/control/SwitchControlActivity.h>
//#include <ui/activities/utilities/AnalogClockActivity.h>
#include <ui/activities/utilities/DigitalClockActivity.h>
#include <ui/activities/utilities/SystemInfoActivity.h>

#include "display/activities/SensorValuesActivity.h"
#include "io/DHTxx.h"
//#include "io/LightSensor.h"

using namespace IO::Env;
using namespace Service;
using namespace UI::Activities::Control;
//using namespace UI::Activities::Examples;
using namespace UI::Activities::Utilities;

HomeGenie* homeGenie;
Module* miniModule;

String controlModuleUrl;
ModuleParameter* controlModuleParameter = nullptr;

SwitchControlActivity* switchControl;

void setup() {

    homeGenie = HomeGenie::getInstance();

    miniModule = homeGenie->getDefaultModule();

    /*
    // Light sensor
    homeGenie->addIOHandler(new LightSensor());
    auto luminance = new ModuleParameter(IOEventPaths::Sensor_Luminance);
    miniModule->properties.add(luminance);
    //*/

    auto roundDisplay = (new UI::Drivers::RoundDisplay())->getDisplay();
    auto dashboard =
        new Dashboard(roundDisplay);

    if (Config::isDeviceConfigured()) {

        // Temperature and humidity sensor
        auto dht = new DHTxx(22);
        homeGenie->addIOHandler(dht);
        auto temperature = new ModuleParameter(IOEventPaths::Sensor_Temperature);
        miniModule->properties.add(temperature);
        auto humidity = new ModuleParameter(IOEventPaths::Sensor_Humidity);
        miniModule->properties.add(humidity);

        // add custom properties
        controlModuleParameter = new ModuleParameter("RemoteControl.EndPoint", Config::getSetting("ctrl-mod"));
        miniModule->properties.add(controlModuleParameter);
        miniModule->properties.add(new ModuleParameter(
                "Widget.OptionField.RemoteControl.EndPoint",
                "module.text:any:switch,light,dimmer,color,shutter:any:uri" // last option can be "uri" or "id"
        ));


        auto systemInfo = new SystemInfoActivity();
        dashboard->addActivity(systemInfo);

        auto digitalClock = new DigitalClockActivity();
        switchControl = new SwitchControlActivity();
        auto sensorValues = new SensorValuesActivity(miniModule);
//      auto analogClock = new AnalogClockActivity();
//      auto gaugeExample = new GaugeExampleActivity();

        dashboard->addActivity(digitalClock);
        dashboard->addActivity(switchControl);
        dashboard->addActivity(sensorValues);
//      dashboard->addActivity(analogClock);
//      dashboard->addActivity(gaugeExample);

        dashboard->setForegroundActivity(sensorValues);

    } else {

        auto systemInfo = new SystemInfoActivity();
        dashboard->addActivity(systemInfo);
        dashboard->setForegroundActivity(systemInfo);

    }

    homeGenie->begin();

}

void loop()
{
    homeGenie->loop();

    if (controlModuleParameter != nullptr && controlModuleParameter->value != controlModuleUrl) {
        controlModuleUrl = controlModuleParameter->value;
        switchControl->setModuleUrl(controlModuleUrl);
        Config::saveSetting("ctrl-mod", controlModuleUrl);
        IO::Logger::info("Control module set to: %s\n", controlModuleUrl.c_str());
    }

}
