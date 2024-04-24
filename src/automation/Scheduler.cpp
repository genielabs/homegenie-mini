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
 * - 2019-01-13 Initial release
 *
 */

#include "Scheduler.h"

#include "ExtendedCron.h"

namespace Automation {
    LinkedList<Schedule*> Scheduler::scheduleList;
    SchedulerListener* Scheduler::listener = nullptr;

    void Scheduler::addSchedule(Schedule* schedule) {
        int existingIndex = -1;
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == schedule->name) {
                existingIndex = i;
                scheduleList.remove(i);
                break;
            }
        }
        scheduleList.add(existingIndex, schedule);
        save();
    }

    void Scheduler::deleteSchedule(const char* name) {
        for (int i = 0; i < scheduleList.size(); i++) {
            auto s = scheduleList.get(i);
            if (s->name == name) {
                scheduleList.remove(i);
                break;
            }
        }
        save();
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

    void Scheduler::loop() {
        auto now = time(0);
        //for(;;) {
        //    int lastRun = millis() % 1000;
            for (int i = 0; i < scheduleList.size(); i++) {
                auto schedule = scheduleList.get(i);
                if (schedule->isEnabled && !schedule->wasScheduled(now) && schedule->occurs(now)) {
                    schedule->setScheduled(now);
                    if (listener != nullptr && schedule->boundModules.size() > 0) {
                        Logger::info(":%s [Scheduler::Event] >> ['%s' triggered]", SCHEDULER_NS_PREFIX,
                                     schedule->name.c_str());
                        listener->onSchedule(schedule);
                    }
                }
            }
        //    int delayMs = 1000 - lastRun;
        //    vTaskDelay(portTICK_PERIOD_MS * delayMs);
        //}
    }

    void Scheduler::load() {
        auto fs = LittleFS;
#ifdef ESP8266
        if(true==fs.begin( )) {
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
                String domain = bm["Domain"].as<const char *>();
                String address = bm["Address"].as<const char *>();
                auto mr = new ModuleReference(domain.c_str(), address.c_str());
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
        JsonArray boundDevices = s[ScheduleField::boundDevices].to<JsonArray>();
        for (int b = 0; b < schedule->boundDevices.size(); b++) {
            auto d = schedule->boundDevices.get(b);
            boundDevices.add(d->c_str());
        }
        JsonArray boundModules = s[ScheduleField::boundModules].to<JsonArray>();
        for (int b = 0; b < schedule->boundModules.size(); b++) {
            auto m = schedule->boundModules.get(b);
            JsonObject module = boundModules.add<JsonObject>();
            module["Domain"] = m->domain;
            module["Address"] = m->address;
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

}