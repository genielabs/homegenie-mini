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
    LinkedList<Schedule*> Scheduler::scheduleList;
    SchedulerListener* Scheduler::listener = nullptr;
    int Scheduler::lastCheckMinute = -1;

    // Adds or updates
    void Scheduler::addSchedule(Schedule* schedule) {
        int existingIndex = -1;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == schedule->name) {
                existingIndex = i;
                auto sc = scheduleList.remove(i);
                delete sc;
                break;
            }
        }
        scheduleList.add(existingIndex, schedule);
        save();
        // update and cache current occurrences
        time_t now = time(0);
        if (schedule->isEnabled) {
            schedule->occursUpdate(now);
        }
    }

    void Scheduler::enableSchedule(const char* name) {
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
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                auto sc = scheduleList.remove(i);
                delete sc;
                save();
                break;
            }
        }
    }

    Schedule* Scheduler::get(const char* name) {
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                return s;
            }
        }
        return nullptr;
    }

    LinkedList<Schedule*> Scheduler::getScheduleList() {
        return scheduleList;
    }

    const int PREEMPTIVE_CHECK_DELAY_MAX = 30000;
    time_t preemptiveDelayMs = PREEMPTIVE_CHECK_DELAY_MAX;
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
    void Scheduler::worker() {
        for(;;) {
            vTaskDelay(1000 * portTICK_PERIOD_MS);

            auto pre_now = time(0) + (preemptiveDelayMs / 1000);
            tm* t = localtime(&pre_now);
            if (t->tm_min == lastCheckMinute || pre_now < Schedule::RTC_SYNC_THRESHOLD) {
                continue;
            }
            lastCheckMinute = t->tm_min;

            unsigned long startEvaluation = millis();
            LinkedList<Schedule*> triggeredSchedule;
            for (int i = 0; i < scheduleList.size(); i++) {
                auto schedule = scheduleList.get(i);
                if (schedule->isEnabled && !schedule->wasScheduled(pre_now) && schedule->occursUpdate(pre_now) && !schedule->onModuleEvent) {
                    triggeredSchedule.add(schedule);
                }
            }

            long actualDelayMs = millis() - startEvaluation;
            auto pause = preemptiveDelayMs - actualDelayMs;
            if (triggeredSchedule.size() && pause > 0) {
                vTaskDelay(pause * portTICK_PERIOD_MS);
            }
            preemptiveDelayMs = actualDelayMs;
            if (preemptiveDelayMs > PREEMPTIVE_CHECK_DELAY_MAX) preemptiveDelayMs = PREEMPTIVE_CHECK_DELAY_MAX;

            auto now = time(0);
            for (int i = 0; i < triggeredSchedule.size(); i++) {
                auto schedule = triggeredSchedule.get(i);
                schedule->setScheduled(now);
                if (listener != nullptr && schedule->boundModules.size() > 0) {
                    Logger::info(":%s [Scheduler::Event] >> ['%s' triggered]", SCHEDULER_NS_PREFIX,
                                 schedule->name.c_str());
                    listener->onSchedule(schedule);
                }
            }
        }
    }
#else
    void Scheduler::loop() {
            auto pre_now = time(0) + (preemptiveDelayMs / 1000);
            tm* t = localtime(&pre_now);
            if (t->tm_min == lastCheckMinute || pre_now < Schedule::RTC_SYNC_THRESHOLD) {
                return;
            }
            lastCheckMinute = t->tm_min;

            unsigned long startEvaluation = millis();
            LinkedList<Schedule*> triggeredSchedule;
            for (int i = 0; i < scheduleList.size(); i++) {
                auto schedule = scheduleList.get(i);
                if (schedule->isEnabled && !schedule->wasScheduled(pre_now) && schedule->occursUpdate(pre_now) && !schedule->onModuleEvent) {
                    triggeredSchedule.add(schedule);
                }
            }

            long actualDelayMs = millis() - startEvaluation;
            auto pause = preemptiveDelayMs - actualDelayMs;
            if (triggeredSchedule.size() && pause > 0) {
                // TODO: long delay should be avoided when not running on a separate task
                delay(pause);
            }
            preemptiveDelayMs = actualDelayMs;
            if (preemptiveDelayMs > PREEMPTIVE_CHECK_DELAY_MAX) preemptiveDelayMs = PREEMPTIVE_CHECK_DELAY_MAX;

            auto now = time(0);
            for (int i = 0; i < triggeredSchedule.size(); i++) {
                auto schedule = triggeredSchedule.get(i);
                schedule->setScheduled(now);
                if (listener != nullptr && schedule->boundModules.size() > 0) {
                    Logger::info(":%s [Scheduler::Event] >> ['%s' triggered]", SCHEDULER_NS_PREFIX,
                                 schedule->name.c_str());
                    listener->onSchedule(schedule);
                }
            }
    }
#endif

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

                addSchedule(getScheduleFromJson(v));

            }

        } else {

            // TODO: report error / disable scheduler

        }
    }

    void Scheduler::save() {
        auto fs = LittleFS;
#ifdef ESP8266
        if(true==fs.begin( )) {
#else
        int maxFiles = 1;
        if(true==fs.begin( true, "/littlefs", maxFiles , "spiffs")) {
#endif
            auto f = LittleFS.open(ScheduleData::fileName, FILE_WRITE);
            f.print(getJsonList());
            f.close();

        } else {

            // TODO: report error / disable scheduler

        }
    }

    Schedule* Scheduler::getScheduleFromJson(JsonVariant v) {

        auto schedule = new Schedule(
                v[ScheduleField::name].as<const char*>(),
                v[ScheduleField::description].as<const char*>(),
                v[ScheduleField::data].as<const char*>(),
                v[ScheduleField::cronExpression].as<const char*>(),
                v[ScheduleField::script].as<const char*>()
        );

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

    String Scheduler::getJsonList() {
        JsonDocument doc;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto schedule = scheduleList.get(i);
            auto jsonSchedule = doc.add<JsonObject>();
            getJson(&jsonSchedule, schedule);
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