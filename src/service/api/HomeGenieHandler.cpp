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
 * - 2019-01-28 Initial release
 *
 */

#include "HomeGenieHandler.h"

bool Service::API::HomeGenieHandler::canHandleDomain(String &domain) {
    return domain == IO::IOEventDomains::HomeAutomation_HomeGenie;
}

bool Service::API::HomeGenieHandler::handleRequest(Service::HomeGenie &homeGenie, Service::APIRequest *request, ESP8266WebServer &server) {
    if (request->Address == "Config") {
        if (request->Command == "Modules.List") {
            // HG Mini multi-sensor module
            auto contentLength = (size_t)homeGenie.writeModuleListJSON(NULL);
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "0");
            server.setContentLength(contentLength);
            server.send(200, "application/json; charset=utf-8", "");
            homeGenie.writeModuleListJSON(&server);
            //server.client().flush();
            return true;
        } else if (request->Command == "Modules.Get") {
            String domain = request->getOption(0);
            String address = request->getOption(1);
            auto contentLength = (size_t)homeGenie.writeModuleJSON(NULL, domain, address);
            if (contentLength == 0) return false;
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "0");
            server.setContentLength(contentLength);
            server.send(200, "application/json; charset=utf-8", "");
            homeGenie.writeModuleJSON(&server, domain, address);
            return true;
        } else if (request->Command == "Groups.List") {
            auto contentLength = (size_t ) homeGenie.writeGroupListJSON(NULL);
            server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
            server.sendHeader("Pragma", "no-cache");
            server.sendHeader("Expires", "0");
            server.setContentLength(contentLength);
            server.send(200, "application/json; charset=utf-8", "");
            homeGenie.writeGroupListJSON(&server);
            //server.client().flush();
            return true;
        }
    }
    return false;
}

bool Service::API::HomeGenieHandler::handleEvent(Service::HomeGenie &homeGenie, IO::IIOEventSender *sender,
                                                 const unsigned char *eventPath, void *eventData,
                                                 IO::IOEventDataType dataType) {

    auto domain = String((char*)sender->getDomain());
    auto address = String((char*)sender->getAddress());
    auto event = String((char*)eventPath);

    // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
    QueuedMessage m = QueuedMessage(domain, address, event, "");
    // Data type handling
    switch (dataType) {
        case SensorLight:
            m.value = String(*(uint16_t *)eventData);
            break;
        case SensorTemperature:
            m.value = String(*(float_t *)eventData);
            break;
        case UnsignedNumber:
            m.value = String(*(uint32_t *)eventData);
            break;
        case Number:
            m.value = String(*(int32_t *)eventData);
            break;
        default:
            m.value = String(*(int32_t *)eventData);
    }
    homeGenie.getEventRouter().signalEvent(m);

    return false;
}
