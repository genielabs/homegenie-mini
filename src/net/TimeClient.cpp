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
 * - 2019-01-10 Initial release
 *
 */

#include "TimeClient.h"

namespace Net {
    // Time sync
    WiFiUDP ntpUDP;
    NTPClient ntpClient(ntpUDP);

    void TimeClient::begin() {
        // Initialize a NTPClient to get time
        ntpClient.begin();
        // Set offset time in seconds to adjust for your timezone, for example:
        // GMT +1 = 3600
        // GMT +8 = 28800
        // GMT -1 = -3600
        // GMT 0 = 0
        ntpClient.setTimeOffset(0);
    }

    void TimeClient::loop() {

#ifndef ESP8266
        // synch with NTP at least once a day
        if (!rtcTimeSet || Config::getRTC()->getLocalEpoch() < 1712031624 || (millis() - lastTimeCheck) > (1440 * 60000))
#endif
        if (WiFi.isConnected() && (millis() - lastTimeCheck) > 60000) {
            if (!ntpClient.isUpdated()) {
                if (ntpClient.update()) {
                    // TimeClient synced with NTP
                    Config::statusLedOff();
#ifndef ESP8266
                    Config::getRTC()->setTime(ntpClient.getEpochTime(), 0);
                    rtcTimeSet = true;
                    Logger::info("|  - RTC updated via TimeClient (NTP)");
#endif
                } else {
                    // NTP Update failed
                    Config::statusLedOn();
                    Logger::warn("|  x TimeClient: NTP update failed!");
                }
            }
            lastTimeCheck = millis();
        }

    }

    void TimeClient::setTime(unsigned long seconds, int ms) {
#ifndef ESP8266
        rtcTimeSet = true;
        Config::getRTC()->setTime(seconds, ms);
        ntpClient.setEpochTime(Config::getRTC()->getLocalEpoch());
#endif
    }

    NTPClient& TimeClient::getNTPClient() {
        return ntpClient;
    }

}
