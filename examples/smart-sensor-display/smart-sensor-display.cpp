/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
#include <ui/drivers/StandardDisplay.h>
#include <ui/activities/utilities/SystemInfoActivity.h>

#ifdef BOARD_HAS_PSRAM
#include <ui/activities/control/SwitchControlActivity.h>
#ifndef DISABLE_LVGL
#include <ui/activities/control/LevelControlActivity.h>
#include <ui/activities/control/ColorControlActivity.h>
#endif
#ifdef LGFX_EXAMPLES
#include <ui/activities/examples/GaugeExampleActivity.h>
#include <ui/activities/examples/AnalogClockActivity.h>
#endif
#include <ui/activities/monitor/CameraDisplayActivity.h>
#include <ui/activities/utilities/DigitalClockActivity.h>
#endif

#include "smart-sensor/CommonSensors.h"

#include "display/activities/SensorValuesActivity.h"

//#include "io/BatterySensor.h"
#include "io/InertialSensor.h"

using namespace Service;

#ifdef BOARD_HAS_PSRAM
using namespace UI::Activities::Control;
#ifdef LGFX_EXAMPLES
using namespace UI::Activities::Examples;
#endif
using namespace UI::Activities::Monitor;
#endif
using namespace UI::Activities::Utilities;

HomeGenie* homeGenie;
Module* miniModule;

Dashboard* dashboard;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Smart Display";

    homeGenie = HomeGenie::getInstance();
    miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty(Implements::Scheduling, "true");
    miniModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

    DisplayDriver* displayDriver;
    auto displayType = Config::getSetting("disp-typ");
    if (displayType.equals("ST7789")) {
        displayDriver = new UI::Drivers::StandardDisplay();
    } else {
        // fallback to "GC9A01"
        displayDriver = new UI::Drivers::RoundDisplay();
    }
    dashboard = new Dashboard(displayDriver);

    if (Config::isDeviceConfigured()) {

        /*
        // Battery sensor
        uint8_t batterySensorPin = 1;
        auto batterySensor = new BatterySensor(batterySensorPin);
        batterySensor->setModule(miniModule);
        homeGenie->addIOHandler(batterySensor);
        //*/
        ///*
        // Inertial sensor
        auto imuType = Config::getSetting("imus-typ");
        if (imuType.equals("QMI8658")) {
            auto inertialSensor = new InertialSensor();
            inertialSensor->setModule(miniModule);
            inertialSensor->onOrientationChange([](InertialSensor::ScreenOrientation orientation) {
                if (orientation <= InertialSensor::ORIENTATION_LANDSCAPE_RIGHT) {
                    Preferences preferences;
                    preferences.begin(CONFIG_SYSTEM_NAME, false);
                    preferences.putUInt(CONFIG_KEY_screen_rotation, orientation);
                    preferences.end();
                    dashboard->setRotation(orientation);
                    dashboard->invalidate();
                }
            });
            homeGenie->addIOHandler(inertialSensor);
        }
        //*/


        includeCommonSensors(homeGenie, miniModule);

#ifdef CONFIG_ENABLE_POWER_MANAGER
        // Enable power manager (deep sleep on motion timeout)
        if (Config::getSetting("motn-typ").equals("switch") && Config::getSetting("motn-pms").equals("sleep")) {
            uint8_t motionSensorPin = Config::getSetting("motn-pin", "16").toInt();
            PowerManager::setWakeUpGPIO((gpio_num_t) motionSensorPin);
        }
#endif

        // Add activities to UI

        // ----------------------------------------------------------------
        // NOTE: Activities can be added if free RAM remains above ~110KB.
        //       This threshold accounts for scheduler and system task
        //       memory requirements.
        // ----------------------------------------------------------------

        // This Activity shows basic system info
        // and buttons to rotate the display
        // or reconfigure the device connection
        auto systemInfo = new SystemInfoActivity();
        dashboard->addActivity(systemInfo);

        // This Activity shows device name, date/time and
        // temperature + humidity if DHT sensor is enabled
        auto sensorValues = new SensorValuesActivity(miniModule);

#ifdef BOARD_HAS_PSRAM
        // A cute "pac-man" clock
        auto digitalClock = new DigitalClockActivity();
        // UI control to switch on/off or set level of
        // user-definable devices. Each button emits events
        // that can be automated using the device Scheduler.
        // It can be configured using HomeGenie Panel app.
        auto switchControl = new SwitchControlActivity("D1");
#ifndef DISABLE_LVGL
        // Similar to the SwitchControl above but using LVGL
        auto levelControl = new LevelControlActivity("M1");
        auto colorControl = new ColorControlActivity("H1");
#endif
        // Displays a remote camera via HTTP images feed,
        // or local camera directly connected to the ESP32.
        auto cameraDisplay = new CameraDisplayActivity("V1");
        if (Config::getSetting(Camera_Sensor::SensorType).equals("esp32-cam")) {
            cameraDisplay->isEsp32Camera = true;
        }
#ifdef LGFX_EXAMPLES
        // UI examples adapted from LoyanGFX library
        auto analogClock = new AnalogClockActivity();
        auto gaugeExample = new GaugeExampleActivity();
#endif

        dashboard->addActivity(digitalClock);
        dashboard->addActivity(switchControl);
#ifndef DISABLE_AUTOMATION
        // Adds scheduler programs to handle on, off and level events
        setupLevelControlActivitySchedule(&switchControl->module);
#endif
#ifndef DISABLE_LVGL
        dashboard->addActivity(levelControl);
        dashboard->addActivity(colorControl);
#ifndef DISABLE_AUTOMATION
        // Adds scheduler programs to handle on, off and level events
        setupLevelControlActivitySchedule(&levelControl->module);
        setupColorControlActivitySchedule(&colorControl->module);
#endif
#endif
        dashboard->addActivity(cameraDisplay);
#ifdef LGFX_EXAMPLES
        dashboard->addActivity(analogClock);
        dashboard->addActivity(gaugeExample);
#endif
#endif

        dashboard->addActivity(sensorValues);

        // Show the SensorValues Activity on start
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
}
