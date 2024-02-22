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
 * - 2019-01-23 Initial release
 *
 */

#include "EventRouter.h"

#include "HomeGenie.h"

namespace Service {

    void EventRouter::loop() {
        if (!WiFi.isConnected()) return;

        // MQTT & SSE Events Queue (dequeue)
        for (int i = 0; i < eventsQueue.size(); i++) {

            auto m = eventsQueue.pop();
            Logger::info(":%s dequeued event >> [domain '%s' address '%s' event '%s']", EVENTROUTER_NS_PREFIX, m.domain.c_str(), m.sender.c_str(), m.event.c_str());
#ifndef DISABLE_MQTT
            // MQTT
            auto date = TimeClient::getTimeClient().getFormattedDate();
            auto topic = String(String(CONFIG_SYSTEM_NAME) + "/" + m.domain + "/" + m.sender + "/event");
            auto details = Service::HomeGenie::createModuleParameter(m.event.c_str(), m.value.c_str(), date.c_str());
            netManager->getMQTTServer().broadcast(&topic, &details);
#endif
            // SSE
            netManager->getHttpServer().sendSSEvent(m.domain, m.sender, m.event, m.value);

            // WS
            if (netManager->getWebSocketServer().connectedClients() > 0) {
                unsigned long epoch = TimeClient::getTimeClient().getEpochTime();
                int ms = TimeClient::getTimeClient().getMilliseconds();
                /*
                // Send as clear text
                int sz = 1+snprintf(nullptr, 0, R"(data: {"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                                    date.c_str(), epoch, ms, m.domain.c_str(), m.sender.c_str(), m.event.c_str(), m.value.c_str());
                char msg[sz];
                snprintf(msg, sz, R"({"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                         date.c_str(), epoch, ms, m.domain.c_str(), m.sender.c_str(), m.event.c_str(), m.value.c_str());
                netManager->getWebSocketServer().broadcastTXT(msg);
                */

                // Send binary packed message

                MsgPack::Packer packer;
                struct timeval tv_now{};
                gettimeofday(&tv_now, nullptr);
                MsgPack::object::timespec t = {
                        .tv_sec  = tv_now.tv_sec, /* int64_t  */
                        .tv_nsec = static_cast<uint32_t>(tv_now.tv_usec) / 10000  /* uint32_t */
                };
                packer.packTimestamp(t);
                auto epochs = String(epoch) + ms;
                packer.packFloat((TimeClient::getTimeClient().getEpochTime() * 1000.0f) + ms);
                packer.pack(m.domain.c_str());
                packer.pack(m.sender.c_str());
                packer.pack("");
                packer.pack(m.event.c_str());
                packer.pack(m.value.c_str());
                netManager->getWebSocketServer().broadcastBIN(packer.data(), packer.size());
            }

            // TODO: route event to the console as well
        }
    }

    void EventRouter::signalEvent(QueuedMessage m) {
        if (WiFi.isConnected()) {
          eventsQueue.add(m);
        }
    }

    void EventRouter::withNetManager(NetManager &manager) {
        netManager = &manager;
    }
}