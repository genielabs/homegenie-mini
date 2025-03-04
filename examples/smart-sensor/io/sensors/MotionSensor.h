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
 * - 2019-01-12 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_MOTIONSENSOR_H
#define HOMEGENIE_MINI_MOTIONSENSOR_H

#include <HomeGenie.h>

#include "../../api/SensorApi.h"

#define MOTION_SENSOR_NS_PREFIX           "IO::Sensors::MotionSensor"

namespace IO { namespace Sensors {

    using namespace Service;
    using namespace SensorApi::Configuration;

    class MotionSensor : Task, public IIOEventSender {
    public:
        explicit MotionSensor(uint8_t pin) {
            setLoopInterval(200);
            sensorPin = pin;
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            m->setProperty(IOEventPaths::Sensor_MotionDetect, "0");
            m->setProperty(IOEventPaths::Status_IdleTime, "0");
            // Add temperature "adjust" UI option:
            // temp. reading offset can be adjusted from -9 to +9
            auto temperatureAdjust = Config::getSetting(DHT_Sensor::TemperatureAdjust, "0");
            m->addWidgetOption(
                    // name, value
                    "Temperature.Adjust", temperatureAdjust.c_str(),
                    // type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":temperature_adjust"
                    // min:max:default
                    ":-9:9:0"
            )->withConfigKey(DHT_Sensor::TemperatureAdjust);
        }
        void begin() override;
        void loop() override;

    private:
        uint8_t sensorPin = 0;
        bool motionDetected = false;
        unsigned long idleTime = 0;
        unsigned long lastUpdate = 0;
        void clearIdle();
        void updateIdle();
    };

}} // Sensors

#endif //HOMEGENIE_MINI_MOTIONSENSOR_H
