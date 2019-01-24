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
 * - 2019-01-23 Initial release
 *
 */

#include <io/Logger.h>
#include "EventRouter.h"
#include "HomeGenie.h"

namespace Service {

    unsigned long lastSendEventTs;

    void EventRouter::loop() {

        if (uptime() > 15000 && millis() - lastSendEventTs > 100) {
            lastSendEventTs = millis();
            // MQTT & SSE Events Queue (dequeue)
            for (int i = 0; i < eventsQueue.size(); i++) {
                // route event through MQTT
                auto m = eventsQueue.pop();
                Logger::trace(":%s de-queued event >> [domain '%s' address '%s' event '%s']", EVENTROUTER_NS_PREFIX, m.domain.c_str(), m.sender.c_str(), m.event.c_str());

                // MQTT
                auto topic = String("hg-mini/"+m.domain+"/" + m.sender + "/event");
                auto details = Service::HomeGenie::createModuleParameter(m.event.c_str(), m.value.c_str());
                netManager->getMQTTServer()->broadcast(&topic, &details);

                // SSE
                netManager->getHttpServer()->sendSSEvent(m.domain, m.sender, m.event, m.value);

                // WS
                if (netManager->getWebSocketServer()->connectedClients() > 0) {
                    String date = Net::NetManager::getTimeClient().getFormattedDate();
                    unsigned long epoch = Net::NetManager::getTimeClient().getEpochTime();
                    int ms = Net::NetManager::getTimeClient().getMilliseconds();
                    int sz = 1+snprintf(NULL, 0, R"(data: {"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                                        date.c_str(), epoch, ms, m.domain.c_str(), m.sender.c_str(), m.event.c_str(), m.value.c_str());
                    char msg[sz];
                    snprintf(msg, sz, R"({"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                             date.c_str(), epoch, ms, m.domain.c_str(), m.sender.c_str(), m.event.c_str(), m.value.c_str());
                    netManager->getWebSocketServer()->broadcastTXT(msg);
                }

                // TODO: route event to the console as well
                // process only one message per cycle to prevent excessive overload
                //break;
            }
        }
    }

    void EventRouter::signalEvent(QueuedMessage m) {
        eventsQueue.add(m);
    }

    void EventRouter::withNetManager(NetManager &manager) {
        netManager = &manager;
    }
}