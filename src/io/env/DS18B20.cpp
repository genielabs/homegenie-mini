/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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
 * - 2019-12-01 Initial release
 *
 */

#include "DS18B20.h"

namespace IO { namespace Env {

    void DS18B20::begin() {
        ds = new OneWire(pinNumber);
        Logger::info("|  ✔ IO::Env::DS18B20");
    }

    void DS18B20::loop() {
        Logger::verbose("  > IO::Env::DS18B20::loop() >> START");

        float temperature = getTemperature();
        Logger::info("@IO::Env::DS18B20 %0.2f", temperature);

        Logger::verbose("  > IO::Env::DS18B20::loop() << END");
    }

    /// Read the temperature value from one DS18S20.
    /// \return The reported temperature in Celsius degrees-
    float DS18B20::getTemperature() {
        //returns the temperature from one DS18S20 in DEG Celsius

        byte data[12];
        byte addr[8];

        if (!ds->search(addr)) {
            //no more sensors on chain, reset search
            ds->reset_search();
            return DS18B20_READ_ERROR;
        }

        if (OneWire::crc8(addr, 7) != addr[7]) {
            Serial.println("CRC is not valid!");
            return DS18B20_READ_ERROR;
        }

        if (addr[0] != 0x10 && addr[0] != 0x28) {
            Serial.print("Device is not recognized");
            return DS18B20_READ_ERROR;
        }

        ds->reset();
        ds->select(addr);
        // start conversion, with parasite power on at the end
        ds->write(0x44, 1);

        // Wait for temperature conversion to complete
        delay(750);

        ds->reset();
        ds->select(addr);
        // Read Scratchpad
        ds->write(0xBE);

        // we need 9 bytes
        for (int i = 0; i < 9; i++) {
            data[i] = ds->read();
        }

        ds->reset_search();

        byte MSB = data[1];
        byte LSB = data[0];

        return ((MSB << 8) | LSB) / 16;
    }

    void DS18B20::setInputPin(const int pinNumber) {
        this->pinNumber = pinNumber;
    }

    void DS18B20::setSamplingRate(const unsigned int samplingRate) {
        setLoopInterval(samplingRate);
    }

}}
