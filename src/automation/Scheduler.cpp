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

#include "Scheduler.h"

#include "ExtendedCron.h"

namespace Automation {
    LinkedList<ScheduleRef> Scheduler::scheduleList;
    SchedulerListener* Scheduler::listener = nullptr;
    int Scheduler::lastCheckMinute = -1;

    // Adds or updates
    bool Scheduler::addSchedule(Schedule* raw) {
        SchedulerLock lock;
        ScheduleRef schedule(raw);
        if (schedule->isEnabled && !schedule->oneTime)
            schedule->occursUpdate(time(nullptr));
        for (int i = 0; i < scheduleList.size(); ++i) {
            auto previous = scheduleList.get(i);
            if (previous->name != schedule->name) continue;
            scheduleList.set(i, schedule);
            if (!save()) {
                scheduleList.set(i, previous);
                return false;
            }
            if (listener && (previous->oneTime || schedule->oneTime))
                listener->onOneTimeChanged(schedule->name);
            return true;
        }
        scheduleList.add(schedule);
        if (!save()) {
            scheduleList.pop();
            return false;
        }
        if (listener && schedule->oneTime) listener->onOneTimeChanged(schedule->name);
        return true;
    }

    void Scheduler::enableSchedule(const char* name) {
        SchedulerLock lock;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                s->isEnabled = true;
                save();
                // update and cache current occurrences
                time_t now = time(0);
                s->occursUpdate(now);
                break;
            }
        }
    }

    void Scheduler::disableSchedule(const char* name) {
        SchedulerLock lock;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                s->isEnabled = false;
                save();
                break;
            }
        }
    }

    void Scheduler::deleteSchedule(const char* name) {
        SchedulerLock lock;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                auto sc = scheduleList.remove(i);
                if (!save()) {
                    scheduleList.add(i, sc);
                    return;
                }
                if (listener && sc->oneTime) listener->onOneTimeChanged(sc->name);
                break;
            }
        }
    }

    ScheduleRef Scheduler::find(const char* name) {
        SchedulerLock lock;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                return s;
            }
        }
        return nullptr;
    }

    LinkedList<ScheduleRef> Scheduler::snapshot() {
        SchedulerLock lock;
        return scheduleList;
    }

    LinkedList<Schedule*> Scheduler::getScheduleList() {
        LinkedList<Schedule*> result;
        for (const auto& item : snapshot()) result.add(item.get());
        return result;
    }

    const int PREEMPTIVE_CHECK_DELAY_MAX = 30000;
    time_t preemptiveDelayMs = PREEMPTIVE_CHECK_DELAY_MAX;
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
    void Scheduler::worker() {
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            tick();
        }
    }
#else
    void Scheduler::loop() { tick(); }
#endif

    void Scheduler::tick() {
        auto pre_now = time(0) + (preemptiveDelayMs / 1000);
        tm* t = localtime(&pre_now);
        if (t->tm_min == lastCheckMinute || pre_now < Schedule::RTC_SYNC_THRESHOLD) {
            return;
        }
        lastCheckMinute = t->tm_min;

        unsigned long startEvaluation = millis();
        LinkedList<ScheduleRef> triggeredSchedule;
        for (auto schedule : snapshot()) {
            if (!schedule->oneTime && schedule->isEnabled && !schedule->wasScheduled(pre_now) && schedule->occursUpdate(pre_now) && !schedule->onModuleEvent) {
                triggeredSchedule.add(schedule);
            }
        }

        long actualDelayMs = millis() - startEvaluation;
        auto pause = preemptiveDelayMs - actualDelayMs;
        if (triggeredSchedule.size() && pause > 0) {
            // TODO: long delay should be avoided when not running on a separate task
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
            vTaskDelay(pdMS_TO_TICKS(pause));
#else
            delay(pause);
#endif
        }
        preemptiveDelayMs = actualDelayMs;
        if (preemptiveDelayMs > PREEMPTIVE_CHECK_DELAY_MAX) preemptiveDelayMs = PREEMPTIVE_CHECK_DELAY_MAX;

        auto now = time(0);
        for (int i = 0; i < triggeredSchedule.size(); i++) {
            auto schedule = triggeredSchedule.get(i);
            if (find(schedule->name.c_str()) != schedule || !schedule->isEnabled) continue;
            schedule->setScheduled(now);
            if (listener != nullptr && schedule->boundModules.size() > 0) {
                Logger::info(":%s [Scheduler::Event] >> ['%s' triggered]", SCHEDULER_NS_PREFIX,
                             schedule->name.c_str());
                listener->onSchedule(schedule.get());
            }
        }
    }

    bool Scheduler::formatLocalMinute(char (&output)[17], const struct tm& local) {
        // One-time schedules compare fixed-width local dates in chronological order.
        return snprintf(output, sizeof(output), "%04d-%02d-%02d %02d:%02d",
                        local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
                        local.tm_hour, local.tm_min) == 16;
    }

    void Scheduler::checkOneTime(time_t now) {
        if (now < Schedule::RTC_SYNC_THRESHOLD || !listener) return;
        struct tm local;
        localtime_r(&now, &local);
        char minute[17];
        if (!formatLocalMinute(minute, local)) return;
        SchedulerLock lock;
        for (int i = scheduleList.size() - 1; i >= 0; --i) {
            auto s = scheduleList.get(i);
            if (!s->oneTime) continue;
            const int cmp = strcmp(s->oneTimeMinute.c_str(), minute);
            if (s->oneTimeFinished || (!s->oneTimeStarted && cmp < 0)) {
                scheduleList.remove(i);
                // Started jobs were already removed from the durable snapshot.
                if (!s->oneTimeStarted && !save()) {
                    scheduleList.add(i, s);
                    continue;
                }
                listener->onOneTimeChanged(s->name);
            } else if (cmp == 0 && s->isEnabled && !s->oneTimeStarted) {
                // Claim durably before dispatch, even if power fails during the script.
                s->oneTimeStarted = true;
                if (save()) listener->onSchedule(s.get());
                else s->oneTimeStarted = false;
            }
        }
    }

    void Scheduler::load() {
        auto fs = LittleFS;
#ifdef ESP8266
        if(true==fs.begin()) {
#else
        int maxFiles = 1;
        if(true == fs.begin(true, "/littlefs", maxFiles , "spiffs") && fs.exists(ScheduleData::fileName)) {
#endif
            auto f = LittleFS.open(ScheduleData::fileName, FILE_READ);
            auto jsonSchedules = f.readString();
            f.close();

            JsonDocument doc;
            deserializeJson(doc, jsonSchedules);

            JsonArray array = doc.as<JsonArray>();
            for(JsonVariant v : array) {

                ScheduleRef schedule(getScheduleFromJson(v));
                if (schedule->isEnabled && !schedule->oneTime)
                    schedule->occursUpdate(time(nullptr));
                scheduleList.add(schedule);

            }

        } else {

            // TODO: report error / disable scheduler

        }
    }

    bool Scheduler::save() {
        SchedulerLock lock;
        const String temp = String(ScheduleData::fileName) + ".tmp";
        auto file = LittleFS.open(temp.c_str(), FILE_WRITE);
        if (!file) return false;
        String snapshot = getJsonList(true);
        bool written = snapshot.length() && file.print(snapshot) == snapshot.length();
        file.close();
        return written && LittleFS.rename(temp.c_str(), ScheduleData::fileName);
    }

    Schedule* Scheduler::getScheduleFromJson(JsonVariant v) {

        auto schedule = new Schedule(
                v[ScheduleField::name].as<const char*>(),
                v[ScheduleField::description].as<const char*>(),
                v[ScheduleField::data].as<const char*>(),
                v[ScheduleField::cronExpression].as<const char*>(),
                v[ScheduleField::script].as<const char*>()
        );

        JsonDocument filter, metadata;
        filter["itemType"] = true;
        filter["from"] = true;
        filter["to"] = true;
        filter["time"][0]["start"] = true;
        filter["time"][0]["end"] = true;
        if (!deserializeJson(metadata, schedule->data, DeserializationOption::Filter(filter)) &&
            metadata["itemType"].as<String>() == "4") {
            // An invalid one-time record must never fall through to recurring cron.
            schedule->oneTime = true;
            String date = metadata["from"] | "";
            String start = metadata["time"][0]["start"] | "";
            if (date.length() == 10 && start.length() == 5 &&
                date == (metadata["to"] | "") && metadata["time"].size() == 1 &&
                start == (metadata["time"][0]["end"] | "")) {
                String stamp = date + " " + start;
                struct tm local{};
                int year, month;
                if (sscanf(stamp.c_str(), "%4d-%2d-%2d %2d:%2d", &year, &month, &local.tm_mday,
                           &local.tm_hour, &local.tm_min) == 5) {
                    local.tm_year = year - 1900;
                    local.tm_mon = month - 1;
                    local.tm_isdst = -1;
                    char normalized[17];
                    if (mktime(&local) != (time_t)-1 &&
                        formatLocalMinute(normalized, local) && stamp == normalized)
                        schedule->oneTimeMinute = stamp;
                }
            }
        }
        if (v.containsKey(ScheduleField::isEnabled)) {
            schedule->isEnabled = v[ScheduleField::isEnabled].as<bool>();
        }

        if (v.containsKey(ScheduleField::onModuleEvent)) {
            schedule->onModuleEvent = v[ScheduleField::onModuleEvent].as<bool>();
        }

        if (v.containsKey(ScheduleField::eventModules)) {
            auto eventModules = v[ScheduleField::eventModules].as<JsonArray>();
            for (auto eventModule: eventModules) {
                auto em = eventModule.as<JsonObject>();
                auto mr = parseModuleReference(em);
                schedule->eventModules.add(mr);
            }
        }
        if (v.containsKey(ScheduleField::boundDevices)) {
            auto boundDevices = v[ScheduleField::boundDevices].as<JsonArray>();
            for (auto boundModule: boundDevices) {
                auto deviceType = new String(boundModule.as<const char*>());
                schedule->boundDevices.add(deviceType);
            }
        }
        if (v.containsKey(ScheduleField::boundModules)) {
            auto boundModules = v[ScheduleField::boundModules].as<JsonArray>();
            for (auto boundModule: boundModules) {
                auto bm = boundModule.as<JsonObject>();
                auto mr = parseModuleReference(bm);
                schedule->boundModules.add(mr);
            }
        }

        return schedule;
    }

    void Scheduler::getJson(JsonObject* jsonSchedule, Schedule* schedule) {
        auto s = (*jsonSchedule);
        s[ScheduleField::name] = schedule->name;
        s[ScheduleField::description] = schedule->description;
        s[ScheduleField::data] = schedule->data;
        s[ScheduleField::cronExpression] = schedule->cronExpression;
        s[ScheduleField::script] = schedule->script;
        s[ScheduleField::isEnabled] = schedule->isEnabled;
        s[ScheduleField::onModuleEvent] = schedule->onModuleEvent;
        JsonArray eventModules = s[ScheduleField::eventModules].to<JsonArray>();
        for (int b = 0; b < schedule->eventModules.size(); b++) {
            auto m = schedule->eventModules.get(b);
            JsonObject mr = eventModules.add<JsonObject>();
            mr["ServiceId"] = m->serviceId;
            mr["Domain"] = m->domain;
            mr["Address"] = m->address;
        }
        JsonArray boundDevices = s[ScheduleField::boundDevices].to<JsonArray>();
        for (int b = 0; b < schedule->boundDevices.size(); b++) {
            auto d = schedule->boundDevices.get(b);
            boundDevices.add(d->c_str());
        }
        JsonArray boundModules = s[ScheduleField::boundModules].to<JsonArray>();
        for (int b = 0; b < schedule->boundModules.size(); b++) {
            auto m = schedule->boundModules.get(b);
            JsonObject mr = boundModules.add<JsonObject>();
            mr["ServiceId"] = m->serviceId;
            mr["Domain"] = m->domain;
            mr["Address"] = m->address;
        }
    }

    String Scheduler::getJsonList(bool forStorage) {
        SchedulerLock lock;
        JsonDocument doc;
        doc.to<JsonArray>();
        for (int i = 0; i < scheduleList.size(); i++) {
            auto schedule = scheduleList.get(i);
            if (forStorage && schedule->oneTimeStarted) continue;
            auto jsonSchedule = doc.add<JsonObject>();
            getJson(&jsonSchedule, schedule.get());
        }
        String output;
        serializeJson(doc, output);
        return output;
    }

    ModuleReference* Scheduler::parseModuleReference(JsonObject& em) {
        String serviceId = "";
        if (em.containsKey("ServiceId")) {
            serviceId = em["ServiceId"].as<const char *>();
        }
        String domain = em["Domain"].as<const char *>();
        String address = em["Address"].as<const char *>();
        return new ModuleReference(serviceId, domain, address);
    }

}