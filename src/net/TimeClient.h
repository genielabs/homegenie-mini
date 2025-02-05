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
 */

#ifndef HOMEGENIE_MINI_TIMECLIENT_H
#define HOMEGENIE_MINI_TIMECLIENT_H

#include "Config.h"

#include <NTPClient.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>

#include <io/Logger.h>
#include "Task.h"

namespace Net {
    using namespace IO;

    class TimeClient: Task {
    public:
        TimeClient() {
            setLoopInterval(5000);
        }
        void begin();
        void loop() override;
        static NTPClient& getNTPClient();
        void setTime(unsigned long seconds, int ms);
        bool isTimeSet() { return rtcTimeSet; };
    private:
#ifdef ESP32
        bool rtcTimeSet = (esp_reset_reason() != ESP_RST_POWERON && esp_reset_reason() != ESP_RST_UNKNOWN);
#else
        bool rtcTimeSet = false;
#endif
        long long lastTimeCheck = -100000;
    };

}

#endif //HOMEGENIE_MINI_TIMECLIENT_H
