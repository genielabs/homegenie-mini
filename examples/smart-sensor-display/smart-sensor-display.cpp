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

#include "smart-sensor/CommonSensors.h"

#include "display/activities/SensorValuesActivity.h"
//#include "io/BatterySensor.h"

// Accelerometer and Gyroscope
//#include "io/QMI8658.h"


using namespace Service;
using namespace UI::Activities::Control;
//using namespace UI::Activities::Examples;
using namespace UI::Activities::Utilities;

HomeGenie* homeGenie;
Module* miniModule;

String controlModuleUrl;
ModuleParameter* controlModuleParameter = nullptr;

SwitchControlActivity* switchControl;


// UI options update listener
static class : public ModuleParameter::UpdateListener {
public:
    void onUpdate(ModuleParameter* option) override {
        Serial.println(option->value);
        controlModuleUrl = option->value;
        switchControl->setModuleUrl(controlModuleUrl);
        Config::saveSetting("ctrl-mod", controlModuleUrl);
        IO::Logger::info("Control module set to: %s", controlModuleUrl.c_str());
    }
} optionUpdateListener;


Dashboard* dashboard;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Smart Sensor Display";

    //uint8_t batterySensorPin = 1;

    homeGenie = HomeGenie::getInstance();
    miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty(Implements::Scheduling, "true");
    miniModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

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


        includeCommonSensors(homeGenie, miniModule);

#ifdef CONFIG_ENABLE_POWER_MANAGER
        // Enable power manager (deep sleep on motion timeout)
        if (Config::getSetting("motn-typ").equals("switch") && Config::getSetting("motn-pms").equals("sleep")) {
            uint8_t motionSensorPin = Config::getSetting("motn-pin", "16").toInt();
            PowerManager::setWakeUpGPIO((gpio_num_t) motionSensorPin);
        }
#endif

        // Add UI control to set the module associated
        // to the light control activity example

        miniModule->addWidgetOption(
            // name, value
            "RemoteControl.EndPoint", "",
                // type
                UI_WIDGETS_FIELD_TYPE_MODULE_TEXT
                // options
                ":any"
                ":switch,light,dimmer,color,shutter,motor"
                ":any"
                ":uri"
        )->withConfigKey("ctrl-mod")->addUpdateListener(&optionUpdateListener);

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
