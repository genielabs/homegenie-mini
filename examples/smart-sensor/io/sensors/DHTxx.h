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

#ifndef HOMEGENIE_MINI_DHTXX_H
#define HOMEGENIE_MINI_DHTXX_H

#include <HomeGenie.h>

#include <dhtnew.h>

#include "../../api/SensorApi.h"

#define DHTXX_NS_PREFIX       "IO::Sensors::DHTxx"
#define SENSOR_SAMPLING_RATE  30000L

namespace IO { namespace Sensors {

    using namespace Service;
    using namespace SensorApi::Configuration;

    class DHTxxSensorData {
    public:
        float_t temperature;
        float_t humidity;
        explicit DHTxxSensorData(DHTxxSensorData* data = nullptr) {
            if (data != nullptr) {
                temperature = data->temperature;
                humidity = data->humidity;
            }
        }
    };

    class DHTxx : Task, public IIOEventSender {
    public:
        float temperatureAdjust = 0;

        DHTxx(uint8_t dhtType, uint8_t pin) {
            setLoopInterval(3000); // initial reading delay
            inputPin = pin;
            dht = new DHTNEW(inputPin);
            dht->setType(dhtType);
            optionUpdateListener = new ParameterListener(this);
        }
        ~DHTxx() {
            delete dht;
            delete optionUpdateListener;
        }
        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            m->setProperty(IOEventPaths::Sensor_Temperature, "0");
            m->setProperty(IOEventPaths::Sensor_Humidity, "0");
            // Add temperature "adjust" UI option:
            // temp. reading offset can be adjusted from -9 to +9
            temperatureAdjust = Config::getSetting(DHT_Sensor::TemperatureAdjust, "0").toFloat();
            m->addWidgetOption(
                    // name, value
                    "Temperature.Adjust", String(temperatureAdjust).c_str(),
                    // type
                    UI_WIDGETS_FIELD_TYPE_NUMBER
                    // label
                    ":temperature_adjust"
                    // min:max:default
                    ":-9:9:0"
            )->withConfigKey(DHT_Sensor::TemperatureAdjust)->addUpdateListener(optionUpdateListener);
        }
        void begin() override;
        void loop() override;
    private:

        class ParameterListener: public ModuleParameter::UpdateListener {
        private:
            DHTxx* dhtSensor;
        public:
            explicit ParameterListener(DHTxx* sc) {
                dhtSensor = sc;
            }
            void onUpdate(ModuleParameter* option) override {
                // The '=' symbol is used here as a special
                // character to separate optional value's
                // language-id string from actual value
                auto v = option->value;
                if (v.indexOf("=") > 0) {
                    v = option->value = v.substring(v.indexOf("=") + 1);
                }
                dhtSensor->temperatureAdjust = v.toFloat();
            }
        };

        // Set DHTxx pin
        uint8_t inputPin = 0;
        // Temperature and humidity sensor
        DHTNEW* dht;
        // Current temperature and humidity
        DHTxxSensorData currentData;
        const float DHT_READ_ERROR = -999;
        void readSensorData();
        ParameterListener* optionUpdateListener;
    };

}}

#endif //HOMEGENIE_MINI_DHTXX_H
