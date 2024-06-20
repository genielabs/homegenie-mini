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

#include "Config.h"

#include <utility>

ZoneConfig Config::zone;
SystemConfig Config::system;
short Config::ServiceButtonPin = -1;
short Config::StatusLedPin = -1;


bool Config::isStatusLedOn = false;
void Config::statusLedOn() {
    isStatusLedOn = true;
    if (Config::StatusLedPin >= 0) digitalWrite(Config::StatusLedPin, HIGH);
    if (ledCallback != nullptr) ledCallback(true);
}
void Config::statusLedOff() {
    isStatusLedOn = false;
    if (Config::StatusLedPin >= 0) digitalWrite(Config::StatusLedPin, LOW);
    if (ledCallback != nullptr) ledCallback(false);
}
std::function<void(bool)> Config::ledCallback = nullptr;
void Config::statusLedCallback(std::function<void(bool)> callback) {
    ledCallback = std::move(callback);
}

void Config::handleConfigCommand(String &message) {
    Preferences preferences;
    preferences.begin(CONFIG_SYSTEM_NAME, false);

    if (message.startsWith("#CONFIG:device-name ")) {
        preferences.putString(CONFIG_KEY_device_name, message.substring(20));
    }
#ifndef CONFIGURE_WITH_WPS
    if (message.startsWith("#CONFIG:wifi-ssid ")) {
        preferences.putString(CONFIG_KEY_wifi_ssid, message.substring(18));
    }
    if (message.startsWith("#CONFIG:wifi-password ")) {
        preferences.putString(CONFIG_KEY_wifi_password, message.substring(22));
        // reset system mode if it was previously forced to "config"
        preferences.putString(CONFIG_KEY_system_mode, "");
    }
#endif
    if (message.startsWith("#CONFIG:system-time ")) {
        String time = message.substring(20);
        long seconds = time.substring(0, time.length() - 3).toInt();
        long ms = time.substring(time.length() - 3).toInt();
#ifndef ESP8266
        Config::getRTC()->setTime(seconds, ms);
#else
        // TODO: not supported by ESP8266
#endif
    }
    if (message.startsWith("#CONFIG:system-zone-id ")) {
        String zoneId = message.substring(23);
        preferences.putString(CONFIG_KEY_system_zone_id, zoneId);
    }
    if (message.startsWith("#CONFIG:system-zone-offset ")) {
        int utcOffset = message.substring(27).toInt(); // minutes
        preferences.putInt(CONFIG_KEY_system_zone_offset, utcOffset);
    }
    if (message.startsWith("#SET:")) {
        int sep = message.substring(5).indexOf(' ') + 5;
        if (sep > 5) {
            auto k = message.substring(5, sep); k.trim();
            auto v = message.substring(sep + 1); v.trim();
            Config::saveSetting(k.c_str(), v);
            Serial.printf("#SET:%s=%s\n", k.c_str(), v.c_str());
        }
    }
    if (message.startsWith("#GET:")) {
        auto k = message.substring(5); k.trim();
        Serial.printf("#GET:%s=%s\n", k.c_str(), Config::getSetting(k.c_str()).c_str());
    }
    preferences.end();
    if (message.equals("#VERSION")) {
        Serial.printf("#VERSION:%s %s (%s %s)\n", CONFIG_SYSTEM_NAME, CONFIG_DEVICE_MODEL_NUMBER, BUILD_ENV_NAME, ReleaseBuildDate);
    } else
    if (message.equals("#RESET")) {
        delay(50);
        ESP.restart();
    }
}
