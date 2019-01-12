//
// Created by gene on 11/01/19.
//

#include "DS18B10.h"

namespace IO { namespace Env {

    void DS18B10::begin() {
        ds = new OneWire(pinNumber);
        Logger::info("|  ✔ IO::Env::DS18B10");
    }

    void DS18B10::loop() {
        static unsigned long lastSampleTime = 0 - samplingRate;
        unsigned long now = millis();
        if (now - lastSampleTime >= samplingRate) {
            float temperature = getTemperature();
            Logger::info("@IO::Env::DS18B10 %0.2f", temperature);
            lastSampleTime = now;
        }
    }

    /// Read the temperature value from one DS18S20.
    /// \return The reported temperature in Celsius degrees-
    float DS18B10::getTemperature() {
        //returns the temperature from one DS18S20 in DEG Celsius

        byte data[12];
        byte addr[8];

        if (!ds->search(addr)) {
            //no more sensors on chain, reset search
            ds->reset_search();
            return DS18B10_READ_ERROR;
        }

        if (OneWire::crc8(addr, 7) != addr[7]) {
            Serial.println("CRC is not valid!");
            return DS18B10_READ_ERROR;
        }

        if (addr[0] != 0x10 && addr[0] != 0x28) {
            Serial.print("Device is not recognized");
            return DS18B10_READ_ERROR;
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

    void DS18B10::setInputPin(const int pinNumber) {
        this->pinNumber = pinNumber;
    }

    void DS18B10::setSamplingRate(const unsigned int samplingRate) {
        this->samplingRate = samplingRate;
    }

}}
