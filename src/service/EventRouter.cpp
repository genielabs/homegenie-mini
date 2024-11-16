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
            auto domain = m.domain.c_str();
            auto sender = m.sender.c_str();
            auto eventPath = m.event.c_str();
            Logger::info(":%s dequeued event >> [domain '%s' address '%s' event '%s']", EVENTROUTER_NS_PREFIX, domain, sender, eventPath);

#ifndef DISABLE_AUTOMATION
            // Run schedules based on module events
            auto scheduleList = Scheduler::getScheduleList();
            for (auto s : scheduleList) {
                if (s->isEnabled && s->onModuleEvent) {

                    bool matchesConditionsProperty = false;

                    // TODO: check if domain/address match declared event modules (`s->eventModules` list)

                    JsonDocument doc;
                    deserializeJson(doc, s->data);
                    auto event = doc["event"].as<JsonArray>();

                    for (auto eventCondition: event) {
                        auto c = eventCondition.as<JsonObject>();

                        // Parse module address
                        // [<server_address_port_or_upnp_uuid>/]<domain>/<address>
                        // examples:
                        // - 34b7da5-2220-6e69-654d-696e69dab734/HomeAutomation.HomeGenie/mini
                        // - 192.168.2.111:8080/HomeAutomation.ZigBee/0017880100B1A9A7
                        // - HomeAutomation.HomeGenie/B1
                        auto moduleId = c["module"].as<String>();
                        int addressIdx = moduleId.lastIndexOf('/');
                        if (addressIdx < 0) continue; // invalid module ID
                        auto moduleAddress = moduleId.substring(addressIdx + 1);
                        auto moduleDomain = moduleId.substring(0, addressIdx);
                        int serverIdx = moduleDomain.lastIndexOf('/');
                        String serverId = "";
                        if (serverIdx >= 0) {
                            serverId = moduleDomain.substring(0, serverIdx);
                            moduleDomain = moduleDomain.substring(serverIdx + 1);
                        }

                        // Only supports events coming from this very system
                        // TODO: this might not work if MAC address changes
//                        if (!serverId.isEmpty() && !serverId.equals(Config::system.id)) {
//                            matchesConditionsProperty = false;
//                            break;
//                        }

                        auto property = c["property"].as<String>();
                        if (strcmp(domain, moduleDomain.c_str()) == 0 &&
                            strcmp(sender, moduleAddress.c_str()) == 0 &&
                            strcmp(eventPath, property.c_str()) == 0) {
                            matchesConditionsProperty = true;
                            break;
                        }
                    }

                    if (matchesConditionsProperty)
                    {

                        bool eventMatchesConditions = false;

                        for (auto eventCondition : event) {
                            auto c = eventCondition.as<JsonObject>();

                            // Parse module address
                            auto moduleId = c["module"].as<String>();
                            int addressIdx = moduleId.lastIndexOf('/');
                            if (addressIdx < 0) continue; // invalid module ID
                            auto moduleAddress = moduleId.substring(addressIdx + 1);
                            auto moduleDomain = moduleId.substring(0, addressIdx);
                            int serverIdx = moduleDomain.lastIndexOf('/');
                            if (serverIdx >= 0) {
                                moduleDomain = moduleDomain.substring(serverIdx + 1);
                            }

                            auto condition = c["condition"].as<String>();
                            auto property = c["property"].as<String>();

                            auto module = HomeGenie::getInstance()->getModule(&moduleDomain, &moduleAddress);
                            if (module != nullptr) {

                                auto mp = module->getProperty(property);
                                if (mp != nullptr && mp->value != nullptr) {

                                    if (Utility::isNumeric(mp->value.c_str())) {
                                        // comparing number values

                                        auto lv = mp->value.toFloat();
                                        auto rv = c["value"].as<float>();
                                        eventMatchesConditions = valueMatchesCondition(condition, lv, rv);

                                    } else {
                                        // comparing string values

                                        auto lv = mp->value;
                                        auto rv = String(c["value"].as<const char*>());
                                        eventMatchesConditions = valueMatchesCondition(condition, lv, rv);

                                    }

                                    // only "AND" logic implemented, all expressions must be true
                                    if (!eventMatchesConditions) break;
                                }

                            } else {

                                // could not find module referenced by expression
                                eventMatchesConditions = false;
                                break;

                            }
                        }

                        if (eventMatchesConditions && (s->cronExpression.isEmpty() || s->info.onThisMinute > 0)) {
                            s->setLastEvent(&m);
                            ProgramEngine::run(s);
                        }

                    }

                }

            }
#endif // #ifndef DISABLE_AUTOMATION


#ifndef DISABLE_MQTT
            // MQTT
            auto date = TimeClient::getTimeClient().getFormattedDate();
            auto topic = String(WiFi.macAddress() + "/" + domain + "/" + sender + "/event");
            auto details = Service::HomeGenie::createModuleParameter(eventPath, m.value.c_str(), date.c_str());
            netManager->getMQTTServer().broadcast(&topic, &details);
#endif
#ifndef DISABLE_SSE
            // SSE
            netManager->getHttpServer().sendSSEvent(domain, sender, eventPath, m.value);
#endif
            // WS
            if (netManager->getWebSocketServer().connectedClients() > 0) {
                unsigned long epoch = TimeClient::getTimeClient().getEpochTime();
                int ms = TimeClient::getTimeClient().getMilliseconds();
                /*
                // Send as clear text
                int sz = 1+snprintf(nullptr, 0, R"(data: {"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                                    date.c_str(), epoch, ms, domain, sender, eventPath, m.value.c_str());
                char msg[sz];
                snprintf(msg, sz, R"({"Timestamp":"%s","UnixTimestamp":%lu%03d,"Description":"","Domain":"%s","Source":"%s","Property":"%s","Value":"%s"})",
                         date.c_str(), epoch, ms, domain, sender, eventPath, m.value.c_str());
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
                packer.packFloat((epoch * 1000.0f) + ms);
                packer.pack(domain);
                packer.pack(sender);
                packer.pack("");
                packer.pack(eventPath);
                packer.pack(m.value.c_str());
                netManager->getWebSocketServer().broadcastBIN(packer.data(), packer.size());
                packer.clear();
            }

            // TODO: route event to the console as well

            yield();
        }
    }

    template <class T>
    bool EventRouter::valueMatchesCondition(String& condition, T leftValue, T rightValue)  {
        bool matchesConditions = false;
        bool isEqualTo = false;
        if (condition.indexOf('=') >= 0) {
            isEqualTo = (leftValue == rightValue);
            matchesConditions = (condition == "!=") ? !isEqualTo : isEqualTo;
        }
        if (condition.startsWith("<") || condition.startsWith(">")) {
            if (condition.startsWith("<")) {
                matchesConditions = (leftValue < rightValue);
            } else if (condition.startsWith(">")) {
                matchesConditions = (leftValue > rightValue);
            }
            if (condition.indexOf('=') > 0) {
                matchesConditions = (matchesConditions || isEqualTo);
            }
        }
        return matchesConditions;
    }

    void EventRouter::signalEvent(QueuedMessage m) {
        if (ESP_WIFI_STATUS == WL_CONNECTED) {
            bool updated = false;
            for (int i = 0; i < eventsQueue.size(); i++) {
                auto qm = eventsQueue.get(i);
                if (qm.domain == m.domain && qm.sender == m.sender && qm.event == m.event && qm.type == m.type) {
                    qm.data = m.data;
                    qm.value = m.value;
                    updated = true;
                    break;
                }
                yield();
            }
            if (!updated) {
                eventsQueue.add(m);
            }
        }
    }

    void EventRouter::withNetManager(NetManager &manager) {
        netManager = &manager;
    }
}