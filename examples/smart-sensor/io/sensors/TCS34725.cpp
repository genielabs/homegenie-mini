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
 * - 2024-10-17 Initial release
 *
 */

#include "TCS34725.h"

void IO::Sensors::TCS34725::begin() {
    // setup color sensor
#ifdef ESP8266
    auto wire = new TwoWire();
    wire->pins(sdaPin, sclPin);
#else
    auto wire = new TwoWire(0);
    wire->setPins(sdaPin, sclPin);
#endif
    if (tcs.begin(TCS34725_ADDRESS, wire)) {
        // Found TCS34725 RGB color sensor.
        initialized = true;
        Logger::info("|  ✔ %s (SDA=%d SCL=%d)", COLOR_SENSOR_NS_PREFIX, sdaPin, sclPin);
    } else {
        // No TCS34725 found.
        Logger::error("|  x %s (SDA=%d SCL=%d)", COLOR_SENSOR_NS_PREFIX, sdaPin, sclPin);
    }
}

void IO::Sensors::TCS34725::loop() {

    if (!initialized) return;

    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    uint16_t lux;
    lux = tcs.calculateLux(r, g, b) * 10; // [0..1000]

    // normalize RGB values
    uint32_t sum = c;
    if (c == 0) {
        r = g = b = 0;
    } else {
        r = round((float)r / (float)sum * 255.0);
        g = round((float)g / (float)sum * 255.0);
        b = round((float)b / (float)sum * 255.0);
    }

    // send event with read values if changed
    if (lux != lightLevel) {
        lightLevel = lux;
        Logger::info("@%s [%s %d]", COLOR_SENSOR_NS_PREFIX, IOEventPaths::Sensor_Luminance, lightLevel);
        sendEvent(IOEventPaths::Sensor_Luminance, &lightLevel, SensorLight);

        auto hsvColor = Utility::rgb2hsv(r, g, b);
        Logger::info("@%s [%s %f,%f,%f]", COLOR_SENSOR_NS_PREFIX, IOEventPaths::Sensor_ColorHsv, hsvColor.h, hsvColor.s, hsvColor.v);
        sendEvent(IOEventPaths::Sensor_ColorHsv, &hsvColor, SensorColorHsv);
    }

    /*
    Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
    Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
    Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
    Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
    Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
    Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
    Serial.println();
    */
}
