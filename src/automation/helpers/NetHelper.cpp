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

#include "NetHelper.h"

namespace Automation { namespace Helpers {

    String NetHelper::httpGet(const String &url) {
        String response;
        auto parsedUrl = Utility::parseURL(url);
        auto client = getClient(parsedUrl.protocol.equalsIgnoreCase("https"));
        HTTPClient http;
        if (http.begin(*client, parsedUrl.url)) {
            if (parsedUrl.hasCredentials()) {
                http.setAuthorization(parsedUrl.username.c_str(), parsedUrl.password.c_str());
            }
            //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int httpCode = http.GET();
            if (httpCode > 0) {
                // Server response
                if (httpCode == HTTP_CODE_OK) {
                    response = http.getString();
                } else {
                    // Server reported error code
                    //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
                }
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
            }
        } else {
            // TODO: report error...
        }
        http.end();
        delete client;
        return response;
    }

    String NetHelper::httpPost(const String& _url, const String& data) {
        String response;
        auto parsedUrl = Utility::parseURL(_url);
        auto client = getClient(parsedUrl.protocol.equalsIgnoreCase("https"));
        HTTPClient http;
        if (http.begin(*client, parsedUrl.url)) {
            if (parsedUrl.hasCredentials()) {
                http.setAuthorization(parsedUrl.username.c_str(), parsedUrl.password.c_str());
            }
            //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int httpCode = http.POST(data);
            if (httpCode > 0) {
                // Server response
                if (httpCode == HTTP_CODE_OK) {
                    response = http.getString();
                } else {
                    // Server reported error code
                    //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
                }
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
            }
        } else {
            // TODO: report error...
        }
        http.end();
        delete client;
        return response;
    }

    bool NetHelper::ping(String &host) {
        return Ping.ping(host.c_str());
    }

    WiFiClient* NetHelper::getClient(bool secure) {
        WiFiClient* client;
        if (secure) {
            client = new WiFiClientSecure();
            ((WiFiClientSecure*)client)->setInsecure();
        } else {
            client = new WiFiClient();
        }
        return client;
    }

}}
