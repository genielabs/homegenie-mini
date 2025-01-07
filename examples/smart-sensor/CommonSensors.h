//
// Created by gene on 21/06/24.
//

#ifndef HOMEGENIE_MINI_COMMONSENSORS_H
#define HOMEGENIE_MINI_COMMONSENSORS_H

#include "HomeGenie.h"

#include "io/sensors/DS18B20.h"
#include "io/sensors/MotionSensor.h"
#include "io/sensors/DHTxx.h"
#include "io/sensors/LightSensor.h"
#include "io/sensors/TCS34725.h"

#include "../color-light/status-led.h"

using namespace IO::Sensors;

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
        s->boundDevices.add(new String("Switch"));
        s->boundDevices.add(new String("Light"));
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
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
        s->boundDevices.add(new String("Switch"));
        s->boundDevices.add(new String("Light"));
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
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
        s->boundDevices.add(new String("Switch"));
        s->boundDevices.add(new String("Light"));
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
        // Custom status led (builtin NeoPixel RGB LED)
        auto pin = Config::getSetting("stld-pin");
        int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
        if (statusLedPin >= 0) {
            s->boundModules.add(new ModuleReference(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS));
        }
        Scheduler::addSchedule(s);
    }

    if (true || Scheduler::getScheduleList().size() == 0) {

        // TODO: if added at least one schedule then  ->  Scheduler::save();

    }

}
#endif

void includeCommonSensors(HomeGenie* homeGenie, Module* sensorModule) {

    // Light sensor
    if (Config::getSetting("ligh-typ").equals("ldr")) {
        auto lightSensor = new LightSensor(Config::getSetting("ligh-pin").toInt());
        lightSensor->setModule(sensorModule);
        homeGenie->addIOHandler(lightSensor);
    }

    // Color sensor TCS-3472x
    if (Config::getSetting("colr-typ").equals("tcs3472")) {
        //TCS34725 colorSensor;
        auto sdaPin = Config::getSetting("colr-sda").toInt();
        auto sclPin = Config::getSetting("colr-scl").toInt();
        auto colorSensor = new TCS34725(sdaPin, sclPin);
        colorSensor->setModule(sensorModule);
        homeGenie->addIOHandler(colorSensor);
    }

    // Motion sensor
    if (Config::getSetting("motn-typ").equals("switch")) {
        uint8_t motionSensorPin = Config::getSetting("motn-pin").toInt();
        auto motionSensor = new MotionSensor(motionSensorPin);
        motionSensor->setModule(sensorModule);
        homeGenie->addIOHandler(motionSensor);
#ifndef DISABLE_AUTOMATION
        // Add example schedules/scenes for motion sensor
        setupMotionSensorSchedules(sensorModule);
#endif
    }

    // Temperature sensor
    if (Config::getSetting("soth-typ").equals("ds18b20")) {
        auto temperatureSensor = new DS18B20(Config::getSetting("soth-pin").toInt());
        temperatureSensor->setModule(sensorModule);
        homeGenie->addIOHandler(temperatureSensor);
    }

    // DHT-xx Temperature and humidity sensor
    uint8_t dhtSensorPint = Config::getSetting("sdht-pin").toInt();
    if (Config::getSetting("sdht-typ").equals("22")) {
        auto dhtSensor = new DHTxx(22, dhtSensorPint);
        dhtSensor->setModule(sensorModule);
        homeGenie->addIOHandler((IIOEventSender*)dhtSensor);
    } else if (Config::getSetting("sdht-typ").equals("11")) {
        auto dhtSensor = new DHTxx(11, dhtSensorPint);
        dhtSensor->setModule(sensorModule);
        homeGenie->addIOHandler((IIOEventSender*)dhtSensor);
    }

}

#endif //HOMEGENIE_MINI_COMMONSENSORS_H
