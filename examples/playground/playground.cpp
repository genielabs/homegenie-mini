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
 * - 2023-12-08 v1,1: added BLE support; targeting ESP32 by default.
 *
 */

#include <HomeGenie.h>

#include <ESP32Servo.h>
Servo myservo;
int pos = 0;

#include "configuration.h"

using namespace IO;
using namespace Service;

HomeGenie* homeGenie;

const int frequency = 50; // Hz      // Standard is 50(hz) servo
const int servoPin = 15;
int minUs = 500;
int maxUs = 2500;

int minWrite = 500; // 0
int maxWrite = 2500; // 180
int inc = 3;
unsigned long lastStepTs = millis();

void setup() {

    homeGenie = HomeGenie::getInstance();

    auto miniModule = homeGenie->getDefaultModule();

    homeGenie->begin();

    myservo.setPeriodHertz(frequency);
    myservo.attach(servoPin, minUs, maxUs);

    //myservo.attach(servoPin);

    //myservo.detach();
}

void loop()
{
    homeGenie->loop();

    if (millis() - lastStepTs > 15) {
        lastStepTs = millis();
        if (pos <= maxWrite) {
            pos += inc;
            Serial.println(pos);
            myservo.write(pos);
        } else { pos = minWrite;
            delay(5000);
        }
    }

}
