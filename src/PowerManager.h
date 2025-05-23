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

#ifndef HOMEGENIE_MINI_POWERMANAGER_H
#define HOMEGENIE_MINI_POWERMANAGER_H

#include "Config.h"

#ifdef ESP32

namespace Service {

    class PowerManager {
    private:
        static unsigned long lastUserActivityTs;
        static unsigned long deepSleepTimeoutMs;
        static unsigned long wakeUpIntervalMs;
        static esp_sleep_source_t lastWakeUpReason;
        static gpio_num_t wakeUpGPIO;
        static touch_pad_t wakeUpTouchPadPin;
        PowerManager() {}

    public:
        static void callback() {}
        static void init();
        static bool canEnterSleepMode();
        static void sleep();
        static void setActive() {
            lastUserActivityTs = millis();
        }
        static void setWakeUpInterval(unsigned long ms) {
            wakeUpIntervalMs = ms;
        }
        static void setWakeUpGPIO(gpio_num_t gpio) {
            wakeUpGPIO = gpio;
        }
        static esp_sleep_source_t getWakeUpReason() {
            auto reason = lastWakeUpReason;
            lastWakeUpReason = ESP_SLEEP_WAKEUP_UNDEFINED;
            return reason;
        }

    };

}

#endif

#endif //HOMEGENIE_MINI_POWERMANAGER_H
