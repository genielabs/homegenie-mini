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

#include <ui/Dashboard.h>
#include <ui/drivers/RoundDisplay.h>
//#include <ui/activities/examples//GaugeExampleActivity.h>
#include <ui/activities/control/SwitchControlActivity.h>
//#include <ui/activities/utilities/AnalogClockActivity.h>
#include <ui/activities/utilities/DigitalClockActivity.h>
#include <ui/activities/utilities/SystemInfoActivity.h>

#include "display/activities/SensorValuesActivity.h"
#include "io/DHTxx.h"
//#include "../smart-sensor/io/LightSensor.h"
//#include "io/BatterySensor.h"
#include "io/MotionSensor.h"

// Accelerometer and Gyroscope
//#include "io/QMI8658.h"


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

Dashboard* dashboard;

void setup() {

    //uint8_t batterySensorPin = 1;
    uint8_t motionSensorPin = 16;

    PowerManager::setWakeUpGPIO((gpio_num_t)motionSensorPin);

    homeGenie = HomeGenie::getInstance();
    miniModule = homeGenie->getDefaultModule();

    /*
    // Init accel./gyro chip
    Wire.begin(DEV_SDA_PIN, DEV_SCL_PIN, 400000);

    QMI8658_enableSensors(QMI8658_CONFIG_AE_ENABLE);
    //QMI8658_enableWakeOnMotion();

    QMI8658_init();
    //*/

    auto roundDisplay = (new UI::Drivers::RoundDisplay())->getDisplay();
    dashboard =
        new Dashboard(roundDisplay);

    if (Config::isDeviceConfigured()) {

        /*
        // Battery sensor
        auto batterySensor = new BatterySensor(batterySensorPin);
        batterySensor->setModule(miniModule);
        homeGenie->addIOHandler(batterySensor);
        //*/

        // Motion sensor
        auto motionSensor = new MotionSensor(motionSensorPin);
        motionSensor->setModule(miniModule);
        homeGenie->addIOHandler(motionSensor);

        /*
        // Light sensor
        auto lightSensor = new LightSensor();
        lightSensor->setModule(miniModule);
        homeGenie->addIOHandler(lightSensor);
        //*/

        // Temperature and humidity sensor
        auto dhtSensor = new DHTxx(22);
        dhtSensor->setModule(miniModule);
        homeGenie->addIOHandler(dhtSensor);

        // add custom properties to default module
        controlModuleParameter = new ModuleParameter("RemoteControl.EndPoint", Config::getSetting("ctrl-mod"));
        miniModule->properties.add(controlModuleParameter);
        miniModule->properties.add(new ModuleParameter(
                "Widget.OptionField.RemoteControl.EndPoint",
                "module.text:any:switch,light,dimmer,color,shutter:any:uri" // last option can be "uri" or "id"
        ));

        // Add activities to UI

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

        // On devices with default RAM, activating
        // Bluetooth will get most of the available RAM,
        // so we just start the configuration activity
        auto systemInfo = new SystemInfoActivity();
        dashboard->addActivity(systemInfo);
        dashboard->setForegroundActivity(systemInfo);

    }

    homeGenie->begin();

}

void loop()
{
    homeGenie->loop();

    // check if configuration parameters changed
    if (controlModuleParameter != nullptr && controlModuleParameter->value != controlModuleUrl) {
        controlModuleUrl = controlModuleParameter->value;
        switchControl->setModuleUrl(controlModuleUrl);
        Config::saveSetting("ctrl-mod", controlModuleUrl);
        IO::Logger::info("Control module set to: %s", controlModuleUrl.c_str());
    }

    /*
    // Test reading accel/gyro data
    float acc[3], gyro[3];
    unsigned int count = 0;
    QMI8658_read_xyz(acc, gyro, &count);
    Serial.printf("ACC X %.2f Y %.2f Z %.2f ", acc[0], acc[1], acc[2]);
    Serial.printf("GYR X %.2f Y %.2f Z %.2f\n", gyro[0], gyro[1], gyro[2]);
    //*/
}
