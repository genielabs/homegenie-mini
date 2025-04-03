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
 * - 2024-06-21 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_COMMONSENSORS_H
#define HOMEGENIE_MINI_COMMONSENSORS_H

#include "../color-light/StatusLed.h"

#include "api/SensorApi.h"
#ifdef ESP_CAMERA_SUPPORTED
#include "examples/smart-sensor/io/sensors/Esp32Camera.h"
#endif
#include "io/sensors/DS18B20.h"
#include "io/sensors/MotionSensor.h"
#include "io/sensors/DHTxx.h"
#include "io/sensors/LightSensor.h"
#include "io/sensors/TCS34725.h"

using namespace IO::Sensors;

using namespace ModuleApi;
using namespace SensorApi::Configuration;

#ifndef DISABLE_AUTOMATION
void setupMotionSensorSchedules(Module* sensorModule) {

    // Create "Motion.Activated" schedule
    if (Scheduler::get("Motion.Activated") == nullptr) {
        auto s = new Schedule("Motion.Activated", "Turn on light if motion is detected and luminosity is low.", "", "", "$$.boundModules.on();");
        s->onModuleEvent = true;
        s->eventModules.add(sensorModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{},"enabled":true,"id":"command_turn_on"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[{"condition":">","module":"HomeAutomation.HomeGenie/mini","property":"Sensor.MotionDetect","value":"0"},{"condition":"<","module":"HomeAutomation.HomeGenie/mini","property":"Sensor.Luminance","value":"70"}],"from":"","itemType":1,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String(ModuleType::Switch));
        s->boundDevices.add(new String(ModuleType::Light));
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
        Scheduler::addSchedule(s);
    }

    // Create "Motion.Timeout" schedule
    if (Scheduler::get("Motion.Timeout") == nullptr) {
        auto s = new Schedule("Motion.Timeout", "Turn off light if no motion is detected for 5 minutes.", "", "", "$$.boundModules.off();");
        s->onModuleEvent = true;
        s->eventModules.add(sensorModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{},"enabled":true,"id":"command_turn_off"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[{"condition":">=","module":"HomeAutomation.HomeGenie/mini","property":"Status.IdleTime","value":"5"},{"condition":">","module":"HomeAutomation.HomeGenie/mini","property":"Sensor.Luminance","value":"50"}],"from":"","itemType":1,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String(ModuleType::Switch));
        s->boundDevices.add(new String(ModuleType::Light));
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
        Scheduler::addSchedule(s);
    }

    // Create "Motion.Blink" schedule
    if (Scheduler::get("Motion.Blink") == nullptr) {
        auto s = new Schedule("Motion.Blink", "Blink light when motion is detected.", "", "", "$$.boundModules.toggle();");
        s->onModuleEvent = true;
        s->eventModules.add(sensorModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{},"enabled":true,"id":"command_toggle"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[{"condition":">=","module":"HomeAutomation.HomeGenie/mini","property":"Sensor.MotionDetect","value":"0"}],"from":"","itemType":1,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String(ModuleType::Switch));
        s->boundDevices.add(new String(ModuleType::Light));
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
        // Custom status led (builtin NeoPixel RGB LED)
        auto pin = Config::getSetting("stld-pin");
        int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
        if (statusLedPin >= 0) {
            s->boundModules.add(new ModuleReference(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS));
        }
        Scheduler::addSchedule(s);
    }
}

void setupCameraSensorSchedules(Module* cameraModule) {

    // Create "Motion.Timeout" schedule
    if (Scheduler::get("Camera.TimeLapse") == nullptr) {
        auto s = new Schedule("Camera.TimeLapse", "Take a picture every minute and save it to SD card.", "", "", "$$.boundModules\n  .command('Camera.FileSave')\n  .submit();\n");
        s->boundModules.add(cameraModule->getReference());
        s->cronExpression = "* * * * *";
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"commands":[{"config":{},"id":"command_camera_filesave","uuid":-4731392927778313137}],"enabled":true,"id":null,"script":null},"forEnd":{"commands":[],"enabled":false,"id":null,"script":null},"forStart":{"commands":[],"enabled":false,"id":null,"script":null}},"type":"template"},"event":[],"from":"","itemType":3,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String(ModuleType::Sensor));
        // Not enabled by default
        s->isEnabled = false;
        Scheduler::addSchedule(s);
    }

}
#endif

void includeCommonSensors(HomeGenie* homeGenie, Module* sensorModule) {

    // Light sensor
    if (Config::getSetting(Light_Sensor::SensorType).equals("ldr")) {
        auto lightSensor = new LightSensor(Config::getSetting(Light_Sensor::SensorPin).toInt());
        lightSensor->setModule(sensorModule);
        homeGenie->addIOHandler(lightSensor);
    }

    // Color sensor TCS-3472x
    if (Config::getSetting(Color_Sensor::SensorType).equals("tcs3472")) {
        //TCS34725 colorSensor;
        auto sdaPin = Config::getSetting(Color_Sensor::SDA).toInt();
        auto sclPin = Config::getSetting(Color_Sensor::SCL).toInt();
        auto colorSensor = new TCS34725(sdaPin, sclPin);
        colorSensor->setModule(sensorModule);
        homeGenie->addIOHandler(colorSensor);
    }

    // Motion sensor
    if (Config::getSetting(Motion_Sensor::SensorType).equals("switch")) {
        uint8_t motionSensorPin = Config::getSetting(Motion_Sensor::SensorPin).toInt();
        auto motionSensor = new MotionSensor(motionSensorPin);
        motionSensor->setModule(sensorModule);
        homeGenie->addIOHandler(motionSensor);
#ifndef DISABLE_AUTOMATION
        // Add example schedules/scenes for motion sensor
        setupMotionSensorSchedules(sensorModule);
#endif
    }

    // Temperature sensor
    if (Config::getSetting(DSB18_Sensor::SensorType).equals("ds18b20")) {
        auto temperatureSensor = new DS18B20(Config::getSetting(DSB18_Sensor::SensorPin).toInt());
        temperatureSensor->setModule(sensorModule);
        homeGenie->addIOHandler(temperatureSensor);
    }

    // DHT-xx Temperature and humidity sensor
    uint8_t dhtSensorPin = Config::getSetting(DHT_Sensor::SensorPin).toInt();
    if (Config::getSetting(DHT_Sensor::SensorType).equals("22")) {
        auto dhtSensor = new DHTxx(22, dhtSensorPin);
        dhtSensor->setModule(sensorModule);
        homeGenie->addIOHandler((IIOEventSender*)dhtSensor);
    } else if (Config::getSetting(DHT_Sensor::SensorType).equals("11")) {
        auto dhtSensor = new DHTxx(11, dhtSensorPin);
        dhtSensor->setModule(sensorModule);
        homeGenie->addIOHandler((IIOEventSender*)dhtSensor);
    }

#ifdef ESP_CAMERA_SUPPORTED
    if (Config::getSetting(Camera_Sensor::SensorType).equals("esp32-cam")) {
        // Add camera handler
        auto cameraHandler = new Camera(IOEventDomains::HomeAutomation_HomeGenie, "Camera", "Camera Sensor");
        homeGenie->addAPIHandler(cameraHandler);
        // Initialize Esp32Camera
        Esp32Camera::init(cameraHandler);
#ifndef DISABLE_AUTOMATION
        // Add example schedule (time-lapse)
        setupCameraSensorSchedules(cameraHandler->getModuleList()->get(0));
#endif
    }
#endif
}

#endif //HOMEGENIE_MINI_COMMONSENSORS_H
