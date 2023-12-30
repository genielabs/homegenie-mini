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
 * - 2019-02-16 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_WIFIMANAGER_H
#define HOMEGENIE_MINI_WIFIMANAGER_H

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#include <Preferences.h>
#endif
#include <WiFiUdp.h>

#include "Config.h"
#include "Task.h"
#include "io/Logger.h"

namespace Net {

    class WiFiManager : Task {
    public:
        WiFiManager();
        void loop() override;
        void initWiFi();
        bool configure();
        bool checkWiFiStatus();
    private:
        wl_status_t wiFiStatus;
    };

}
#endif //HOMEGENIE_MINI_WIFIMANAGER_H
