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
