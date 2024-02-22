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
 * - 2019-01-12 Initial release
 *
 */

#include "DHTxx.h"

namespace IO { namespace Env {

    void DHTxx::begin() {
        dht->setDisableIRQ(true);
        Logger::info("|  ✔ %s (PIN=%d)", DHTXX_NS_PREFIX, inputPin);
    }

    void DHTxx::loop() {
        Logger::verbose("  > %s::loop() >> START", DHTXX_NS_PREFIX);

        float_t t = currentData.temperature;
        float_t h = currentData.humidity;
        readSensorData();
        // signal value changes
        if (currentData.temperature != t) {
            Logger::info("@%s [%s %0.2f]", DHTXX_NS_PREFIX, (IOEventPaths::Sensor_Temperature), currentData.temperature);
            sendEvent((const uint8_t*)(IOEventPaths::Sensor_Temperature), (float_t *)&currentData.temperature, SensorTemperature);
        }
        if (currentData.humidity != h) {
            Logger::info("@%s [%s %0.2f]", DHTXX_NS_PREFIX, (IOEventPaths::Sensor_Humidity), currentData.humidity);
            sendEvent((const uint8_t*)(IOEventPaths::Sensor_Humidity), (float_t *)&currentData.humidity, SensorHumidity);
        }

        Logger::verbose("  > %s::loop() << END", DHTXX_NS_PREFIX);
        setLoopInterval(SENSOR_SAMPLING_RATE);
    }

    /// Read temperature and humidity values from one DHTxx.
    void DHTxx::readSensorData() {
        uint8_t attempts = 5;
        while (attempts > 0) {
            dht->read();
            float h = dht->getHumidity();
            float t = dht->getTemperature();
            if (h != DHT_READ_ERROR && t != DHT_READ_ERROR) {
                currentData.temperature = t;
                currentData.humidity = h;
                break;
            }
            // TODO: report error reading sensor data
            attempts--;
        }
    }

}}
