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

#ifdef ESP32

#include "PowerManager.h"

#if CONFIG_IDF_TARGET_ESP32
    #define THRESHOLD   40      /* Greater the value, more the sensitivity */
#else //ESP32-S2 and ESP32-S3 + default for other chips (to be adjusted) */
    #define THRESHOLD   5000   /* Lower the value, more the sensitivity */
#endif

RTC_DATA_ATTR int wakeUpCount = 0;

namespace Service {
    unsigned long PowerManager::lastUserActivityTs = 0;
    unsigned long PowerManager::deepSleepTimeoutMs = 30000;
    // wake up sources
    unsigned long PowerManager::wakeUpIntervalMs = 0;
    gpio_num_t PowerManager::wakeUpGPIO = GPIO_NUM_NC;
    touch_pad_t PowerManager::wakeUpTouchPadPin = TOUCH_PAD_NUM0;

    void PowerManager::init() {

        if (wakeUpCount > 0) {

            esp_sleep_wakeup_cause_t wakeup_reason;
            wakeup_reason = esp_sleep_get_wakeup_cause();
            switch(wakeup_reason)
            {
                case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
                case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
                case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
                case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
                case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
                default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
            }

        } else {

            // initial setup
            lastUserActivityTs = millis();

        }

        wakeUpCount++;
    }

    bool PowerManager::canEnterSleepMode() {
        return (millis() - lastUserActivityTs > deepSleepTimeoutMs);
    }

    void PowerManager::sleep() {

        // Configure wake up sources

        // Wakeup on touchpad activity
// TODO:       touchSleepWakeUpEnable(wakeUpTouchPadPin, 40);

        // Wakeup on external GPIO high
        if (wakeUpGPIO != GPIO_NUM_NC) {
#ifndef ESP32_C3
            esp_sleep_enable_ext0_wakeup(wakeUpGPIO, ESP_EXT1_WAKEUP_ANY_HIGH);
#else
            esp_deep_sleep_enable_gpio_wakeup(wakeUpGPIO, ESP_GPIO_WAKEUP_GPIO_HIGH);
#endif
        }

        // Wakeup after a given interval
        if (wakeUpIntervalMs > 0) {
            esp_sleep_enable_timer_wakeup(wakeUpIntervalMs * 1000);
        }

        // Enter deep sleep mode
        esp_deep_sleep_start();
//            esp_light_sleep_start();

    }

}

#endif