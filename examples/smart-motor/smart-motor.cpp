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
#include <service/api/devices/Dimmer.h>

#include "api/MotorHandler.h"

#ifdef BOARD_HAS_RGB_LED
#include "../color-light/StatusLed.h"
StatusLed statusLed;
#endif

using namespace IO;
using namespace Service;
using namespace Service::API::devices;

HomeGenie* homeGenie;

MotorHandler* motorHandler;
bool is4wdRcCar;
bool isRoboticArm;
bool roboArmInitialized = false;

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "Smart Motor";

    homeGenie = HomeGenie::getInstance();

#ifdef BOARD_HAS_RGB_LED
    // Custom status led (builtin NeoPixel RGB LED)
    statusLed.setup();
#endif

    motorHandler = new MotorHandler();
    homeGenie->addAPIHandler(motorHandler);

    homeGenie->begin();

    // Adds custom control modules

    is4wdRcCar = Config::getSetting("rc-car-4wd").equals("true");
    if (is4wdRcCar) {

        // Add *Motion* and *Steering* control modules if this device is for the
        // FPV 4WD RC Car https://homegenie.it/mini/1.2/examples/fpv-rc-car/

        //   RC Car top view:
        //
        //   (S4)-----(S3)   - front wheels
        //      | ... |
        //      | ... |
        //   (S2)-----(S1)   - rear wheels (inverted mounting direction)
        //
        // - S1 = Servo Motor 1
        // - S2 = Servo Motor 2
        // - S3 = Servo Motor 3
        // - S4 = Servo Motor 4

        auto list = motorHandler->getModuleList();
        if (list->size() >= 4) {
            for (auto i = 0; i < 4; i++) {
                list->get(i)->setProperty(WidgetApi::Preference::HomeLevel, "50");
                list->get(i)->setProperty(WidgetApi::Preference::AutoHome, "true");
            }
            // Module S1 to S4 are the 4WD motors
            auto motionControl = new Dimmer(IO::IOEventDomains::Automation_Components, "MT1", "Motion Control");
            motionControl->module->type = ModuleType::Motor;
            motionControl->module->setProperty(WidgetApi::DisplayModule, "homegenie/generic/shutter");
            motionControl->module->setProperty(WidgetApi::Preference::HomeLevel, "50");
            motionControl->module->setProperty(WidgetApi::Preference::AutoHome, "true");
            motionControl->module->setProperty(WidgetApi::Preference::LevelStep, "2");
            motionControl->setDefaultTransition(0);
            motionControl->setLevel(0.5f);
            motionControl->onSetLevel([list](float level) {
                static float currentLevel = 0;
                if (level == currentLevel) {
                    return;
                }
                currentLevel = level;
                // move all 4WD motors
                //       > 0.5 to 1.0 forward
                //       < 0.5 to 0.0 backward
                //       = 0.5 stopped
                auto s1 = list->get(0);
                auto s2 = list->get(1);
                auto s3 = list->get(2);
                auto s4 = list->get(3);
                level *= 100;
                ((MotorModule*) s1)->motorControl->setLevel(100 - level);
                ((MotorModule*) s2)->motorControl->setLevel(level);
                ((MotorModule*) s3)->motorControl->setLevel(100 - level);
                ((MotorModule*) s4)->motorControl->setLevel(level);
            });
            HomeGenie::getInstance()->addAPIHandler(motionControl);
            auto steerControl = new Dimmer(IO::IOEventDomains::Automation_Components, "ST1", "Steering Control");
            steerControl->module->type = ModuleType::Motor;
            steerControl->module->setProperty(WidgetApi::DisplayModule, "homegenie/generic/shutter");
            steerControl->module->setProperty(WidgetApi::Preference::HomeLevel, "50");
            steerControl->module->setProperty(WidgetApi::Preference::AutoHome, "true");
            steerControl->module->setProperty(WidgetApi::Preference::LevelStep, "2");
            steerControl->setDefaultTransition(0);
            steerControl->setLevel(0.5f);
            steerControl->onSetLevel([list](float level) {
                static float currentLevel = 0;
                Serial.println(level);
                if (level == currentLevel) {
                    return;
                }
                currentLevel = level;
                // Steering system
                //       > 0.5 to 1.0 - steer right : move S1,S3 backward and S2,S4 forward
                //       < 0.5 to 0.0 - steer left  : move S1,S3 forward  and S2,S4 backward
                //       = 0.5 stopped
                auto s1 = list->get(0);
                auto s2 = list->get(1);
                auto s3 = list->get(2);
                auto s4 = list->get(3);
                level *= 100;
                ((MotorModule*) s1)->motorControl->setLevel(level);
                ((MotorModule*) s2)->motorControl->setLevel(level);
                ((MotorModule*) s3)->motorControl->setLevel(level);
                ((MotorModule*) s4)->motorControl->setLevel(level);
            });
            HomeGenie::getInstance()->addAPIHandler(steerControl);

            // initialize motors
            auto arm = std::vector<String>{"S1", "S2", "S3", "S4"};
            for (const auto &addr: arm) {
                auto sm = motorHandler->getModule(IO::IOEventDomains::Automation_Components, addr.c_str());
                if (sm) {
                    // "10" = max position seeking speed  (1 - 10)
                    ((MotorModule*) sm)->setProperty(Options::Motor_Servo_AngleSpeed, "10");
                    // position to 50 (rest position)
                    ((MotorModule*) sm)->motorControl->setLevel(50); // (0-100)
                }
            }
        }
    }

    isRoboticArm = Config::getSetting("robotc-arm").equals("true");
    if (isRoboticArm) {
        // If this device is for the Robotic Arm only it will be using modules "S1" to "S4".
        // If this device is for RC-CAR with Robotic Arm, arm modules will be "S5" to "S8".
        // Configure Robotic Arm initial position to 0.5
        auto arm = !is4wdRcCar ? std::vector<String>{"S1", "S2", "S3", "S4"} : std::vector<String>{"S5", "S6", "S7", "S8"};
        for (const auto &addr: arm) {
            auto sm = motorHandler->getModule(IO::IOEventDomains::Automation_Components, addr.c_str());
            if (sm) {
                // max speed to seek initial position
                ((MotorModule *) sm)->setProperty(Options::Motor_Servo_AngleSpeed, "10");
                // position to 50 (rest position)
                ((MotorModule *) sm)->motorControl->setLevel(50); // (0-100)
            }
        }
    }

}

void loop()
{
    homeGenie->loop();

    if (isRoboticArm && !roboArmInitialized && millis() > 3000) {
        roboArmInitialized = true;
        auto arm = !is4wdRcCar ? std::vector<String>{"S1", "S2", "S3", "S4"} : std::vector<String>{"S5", "S6", "S7", "S8"};
        for (const auto &addr: arm) {
            auto sm = motorHandler->getModule(IO::IOEventDomains::Automation_Components, addr.c_str());
            if (sm) {
                // set slower speed for normal operations (smoother movements)
                sm->setProperty(Options::Motor_Servo_AngleSpeed, "5");
            }
        }
    }
}
