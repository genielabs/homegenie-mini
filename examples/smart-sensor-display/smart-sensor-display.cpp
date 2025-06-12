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

#include "configuration.h"

HomeGenie* homeGenie;
Module* miniModule;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Smart Display";

    homeGenie = HomeGenie::getInstance();
    miniModule = homeGenie->getDefaultModule();
    miniModule->setProperty(Implements::Scheduling, "true");
    miniModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

    DisplayDriver* displayDriver = nullptr;
    auto displayType = Config::getSetting("disp-typ");
    if (displayType.equals("ST7789")) {
        // ST7789 320x240 display
        displayDriver = new UI::Drivers::ST7789();
    } else if (displayType.equals("ST7796")) {
        // ST7789 480x320 display
        displayDriver = new UI::Drivers::ST7796();
    } else if (displayType.equals("GC9A01")) {
        // GC9A01 240x240 round display
        displayDriver = new UI::Drivers::GC9A01();
    } else if (displayType.equals("AUTO")) {
        // fallback to autodetect
        displayDriver = new UI::Drivers::AutodetectDisplay();
    }

    if (displayDriver) {
        initDashboard(displayDriver, miniModule);
    } else {
        Serial.println("ERROR: display driver not set, could not initialize Dashboard.");
    }

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

        if (dashboard) {

            auto dashboardConfig = Config::getSetting(
                    "dashboard",
#ifdef BOARD_HAS_PSRAM
                    "SensorValues,CameraDisplay:V1,LevelControl:M1,ColorControl:H1,SwitchControl:D1,DigitalClock,"
                    #ifdef LGFX_EXAMPLES
                    "AnalogClock,GaugeExample,"
                    #endif
                    "SystemInfo"
#else
                    "SensorValues,SystemInfo"
#endif
            );
            // very basic consistency check
            dashboardConfig.trim();
            if (dashboardConfig.isEmpty()) {
                // ensure there is at least the SystemInfo activity
                dashboardConfig = "SystemInfo";
            }

            // ----------------------------------------------------------------
            // NOTE: Activities can be added if free RAM remains above ~110KB.
            //       This threshold accounts for scheduler and system task
            //       memory requirements.
            // ----------------------------------------------------------------

            // Add activities to UI
            addDashboardActivities(dashboardConfig);
        }

    } else if (dashboard) {

        // On devices with default RAM, activating
        // Bluetooth will get most of the available RAM,
        // so we just start the configuration activity
        auto systemInfo = new SystemInfoActivity();
        dashboard->addActivity(systemInfo);

    }

    homeGenie->begin();
}

void loop()
{
    homeGenie->loop();
}
