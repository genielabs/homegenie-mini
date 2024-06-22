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

using namespace IO::Sensors;

void includeCommonSensors(HomeGenie* homeGenie, Module* miniModule) {

    // Light sensor
    if (Config::getSetting("ligh-typ").equals("ldr")) {
        auto lightSensor = new LightSensor(Config::getSetting("ligh-pin").toInt());
        lightSensor->setModule(miniModule);
        homeGenie->addIOHandler(lightSensor);
    }

    // Motion sensor
    if (Config::getSetting("motn-typ").equals("switch")) {
        uint8_t motionSensorPin = Config::getSetting("motn-pin").toInt();
        auto motionSensor = new MotionSensor(motionSensorPin);
        motionSensor->setModule(miniModule);
        homeGenie->addIOHandler(motionSensor);
    }

    // Temperature sensor
    if (Config::getSetting("soth-typ").equals("ds18b20")) {
        auto temperatureSensor = new DS18B20(Config::getSetting("soth-pin").toInt());
        temperatureSensor->setModule(miniModule);
        homeGenie->addIOHandler(temperatureSensor);
    }

    // DHT-xx Temperature and humidity sensor
    uint8_t dhtSensorPint = Config::getSetting("sdht-pin").toInt();
    if (Config::getSetting("sdht-typ").equals("22")) {
        auto dhtSensor = new DHTxx(22, dhtSensorPint);
        dhtSensor->setModule(miniModule);
        homeGenie->addIOHandler(dhtSensor);
    } else if (Config::getSetting("sdht-typ").equals("11")) {
        auto dhtSensor = new DHTxx(11, dhtSensorPint);
        dhtSensor->setModule(miniModule);
        homeGenie->addIOHandler(dhtSensor);
    }
}

#endif //HOMEGENIE_MINI_COMMONSENSORS_H
