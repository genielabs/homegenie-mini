/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
 * - 2019-01-13 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_NETHELPER_H
#define HOMEGENIE_MINI_NETHELPER_H

#include <WiFiClientSecure.h>

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#else
#include <HTTPClient.h>
#endif
#include <ESPping.h>

#include "Config.h"

namespace Automation { namespace Helpers {

    class NetHelper {
    public:
        static String httpGet(String& url);
        static String httpPost(String& url, String& data);
        static bool ping(String& host);

    private:
        static WiFiClientSecure* wifiClientSecure;
        static HTTPClient http;

        static WiFiClient* getClient(String& url);
    };

}}

#endif //HOMEGENIE_MINI_NETHELPER_H
