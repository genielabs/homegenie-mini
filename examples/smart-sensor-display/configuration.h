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
 */

#ifndef HOMEGENIE_MINI_CONFIGURATION_H
#define HOMEGENIE_MINI_CONFIGURATION_H

#include "defs.h"

#include <ui/Dashboard.h>
#include <ui/drivers/GC9A01.h>
#include <ui/drivers/ST7789.h>
#include <ui/drivers/ST7796.h>
#include <ui/drivers/AutodetectDisplay.h>
#include <ui/activities/utilities/SystemInfoActivity.h>

#ifdef BOARD_HAS_PSRAM
#include <ui/activities/control/SwitchControlActivity.h>
#ifndef DISABLE_LVGL
#include <ui/activities/control/ColorControlActivity.h>
#include <ui/activities/control/LevelControlActivity.h>
#endif
#include <ui/activities/examples/AnalogClockActivity.h>
#include <ui/activities/examples/GaugeExampleActivity.h>
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
using namespace UI::Activities::Examples;
using namespace UI::Activities::Monitor;
#endif
using namespace UI::Activities::Utilities;

namespace DisplayConfig = DisplayApi::Configuration;
namespace DisplayOption = DisplayApi::Options;


Dashboard* dashboard = nullptr;


// UI options update listener
static class : public ModuleParameter::UpdateListener {
public:
    void onUpdate(ModuleParameter* option) override {
#ifdef ENABLE_SCREEN_SAVER
        if (option->is(DisplayOption::Display_ScreenSaverTimeout)) {
            dashboard->setScreenSaverTimeout(option->value.toInt() * 1000);
        }
#endif
        if (option->is(DisplayOption::Display_Brightness)) {
            dashboard->setBrightness(option->value.toInt());
        }
    }
} dashboardOptionUpdateListener;


void initDashboard(DisplayDriver* displayDriver, Module* module) {
    dashboard = new Dashboard(displayDriver);
#ifdef ENABLE_SCREEN_SAVER
    auto timeoutMs = Config::getSetting(DisplayConfig::ScreenSaver::TimeoutSeconds, "15");
    int screenSaverTimeoutMs = timeoutMs.toInt() * 1000;
    dashboard->setScreenSaverTimeout(screenSaverTimeoutMs);
    module->addWidgetOption(
            // name, value
            DisplayOption::Display_ScreenSaverTimeout, timeoutMs.c_str(),
            // type
            UI_WIDGETS_FIELD_TYPE_SLIDER
            // label
            ":screensaver_timeout"
            // min:max:default
            ":0:120:15"
    )->withConfigKey(DisplayConfig::ScreenSaver::TimeoutSeconds)->addUpdateListener(&dashboardOptionUpdateListener);
#endif
    module->addWidgetOption(
            // name, value
            DisplayOption::Display_Brightness, timeoutMs.c_str(),
            // type
            UI_WIDGETS_FIELD_TYPE_SLIDER
            // label
            ":display_brightness"
            // min:max:default
            ":1:127:64"
    )->withConfigKey(DisplayConfig::Display::Brightness)->addUpdateListener(&dashboardOptionUpdateListener);
}

void addDashboardActivities(String &list) {
    int currentIndex = 0;
    int nextCommaIndex = 0;

    while (currentIndex < list.length()) {
        nextCommaIndex = list.indexOf(',', currentIndex);

        String currentElement;
        if (nextCommaIndex == -1) {
            currentElement = list.substring(currentIndex);
            currentIndex = list.length(); // exit loop
        } else {
            currentElement = list.substring(currentIndex, nextCommaIndex);
            currentIndex = nextCommaIndex + 1;
        }
        currentElement.trim();
        if (currentElement.length() > 0) {

            String elementName;
            String elementOptions = "";
            int colonIndex = currentElement.indexOf(':');
            if (colonIndex == -1) {
                elementName = currentElement;
            } else {
                elementName = currentElement.substring(0, colonIndex);
                elementOptions = currentElement.substring(colonIndex + 1);
            }
            elementName.trim();
            elementOptions.trim();

            // This Activity shows basic system info
            // and buttons to rotate the display
            // or reconfigure the device connection
            static int systemInfoCount = 0;
            if (elementName == "SystemInfo") {
                // single instance activity
                if (++systemInfoCount == 1) {
                    auto systemInfo = new SystemInfoActivity();
                    dashboard->addActivity(systemInfo);
                }
            }

            // This Activity shows device name, date/time and
            // temperature + humidity if DHT sensor is enabled
            static int sensorValuesCount = 0;
            if (elementName == "SensorValues") {
                // single instance activity
                if (++sensorValuesCount == 1) {
                    auto miniModule = HomeGenie::getInstance()->getDefaultModule();
                    auto sensorValues = new SensorValuesActivity(miniModule);
                    dashboard->addActivity(sensorValues);
                }
            }

#ifdef BOARD_HAS_PSRAM
            // A cute "pac-man" clock
            static int digitalClockCount = 0;
            if (elementName == "DigitalClock") {
                // single instance activity
                if (++digitalClockCount == 1) {
                    auto digitalClock = new DigitalClockActivity();
                    dashboard->addActivity(digitalClock);
                }
            }

            // UI control to switch on/off or set level of
            // user-definable devices. Each button emits events
            // that can be automated using the device Scheduler.
            // It can be configured using HomeGenie Panel app.
            static int switchControlCount = 0;
            if (elementName == "SwitchControl") {
                String address = "D" + String(++switchControlCount);
                if (elementOptions.length() > 0) {
                    address = elementOptions;
                }
                auto switchControl = new SwitchControlActivity(address.c_str());
                dashboard->addActivity(switchControl);
#ifndef DISABLE_AUTOMATION
                // Adds scheduler programs to handle Level events
                setupLevelControlActivitySchedule(&switchControl->module);
#endif
            }

#ifndef DISABLE_LVGL
            // Similar to the SwitchControl but using LVGL
            static int levelControlCount = 0;
            if (elementName == "LevelControl") {
                String address = "M" + String(++levelControlCount);
                if (elementOptions.length() > 0) {
                    address = elementOptions;
                }
                auto levelControl = new LevelControlActivity(address.c_str());
                dashboard->addActivity(levelControl);
#ifndef DISABLE_AUTOMATION
                // Adds scheduler programs to handle Level events
                setupLevelControlActivitySchedule(&levelControl->module);
#endif
            }
            // Color control
            static int colorControlCount = 0;
            if (elementName == "ColorControl") {
                String address = "H" + String(++colorControlCount);
                if (elementOptions.length() > 0) {
                    address = elementOptions;
                }
                auto colorControl = new ColorControlActivity(address.c_str());
                dashboard->addActivity(colorControl);
#ifndef DISABLE_AUTOMATION
                // Adds scheduler programs to handle ColorHsv events
                setupColorControlActivitySchedule(&colorControl->module);
#endif
            }
#endif // DISABLE_LVGL
            // Displays a remote camera via HTTP images feed,
            // or local camera directly connected to the ESP32.
            static int cameraDisplayCount = 0;
            if (elementName == "CameraDisplay") {
                String address = "V" + String(++cameraDisplayCount);
                if (elementOptions.length() > 0) {
                    address = elementOptions;
                }
                auto cameraDisplay = new CameraDisplayActivity(address.c_str());
                if (cameraDisplayCount == 1) {
                    // The first instance is reserved for
                    // onboard ESP32 camera module if enabled
                    if (Config::getSetting(Camera_Sensor::SensorType).equals("esp32-cam")) {
                        cameraDisplay->isEsp32Camera = true;
                    }
                }
                dashboard->addActivity(cameraDisplay);
            }

            // UI examples adapted from LovyanGFX library
            static int analogClockCount = 0;
            if (elementName == "AnalogClock") {
                // single instance activity
                if (++analogClockCount == 1) {
                    auto analogClock = new AnalogClockActivity();
                    dashboard->addActivity(analogClock);
                }
            }
            static int gaugeExampleCount = 0;
            if (elementName == "GaugeExample") {
                // single instance activity
                if (++gaugeExampleCount == 1) {
                    auto gaugeExample = new GaugeExampleActivity();
                    dashboard->addActivity(gaugeExample);
                }
            }

#endif // BOARD_HAS_PSRAM
        }
    }

}

#endif //HOMEGENIE_MINI_CONFIGURATION_H
