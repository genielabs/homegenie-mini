/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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

#ifndef HOMEGENIE_MINI_WIFIMANAGER_H
#define HOMEGENIE_MINI_WIFIMANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>

#include "HTTPServer.h"
#include "MQTTServer.h"
#include <io/Logger.h>
#include <Task.h>

#define NETMANAGER_LOG_PREFIX           "@Net::NetManager"

namespace Net {

    /// Network services management
    class NetManager : Task {
    public:
        NetManager();
        ~NetManager();
        bool begin();
        void loop();
        HTTPServer getHttpServer();
        MQTTServer getMQTTServer();

    private:
        HTTPServer *httpServer;
        MQTTServer *mqttServer;
        String setStatus(int s);
    };

}

#endif //HOMEGENIE_MINI_WIFIMANAGER_H
