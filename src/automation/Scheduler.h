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

#ifndef HOMEGENIE_MINI_SCHEDULER_H
#define HOMEGENIE_MINI_SCHEDULER_H

#include <ArduinoJson.h>
#include <LinkedList.h>
#include <LittleFS.h>

#include "Task.h"
#include "lib/supertinycron/ccronexpr.h"
#include "data/Module.h"

#define SCHEDULER_NS_PREFIX            "Automation::Scheduler"

namespace Automation {

    using namespace Data;

    namespace ScheduleField {
        static const char name[] PROGMEM = "Name";
        static const char description[] PROGMEM = "Description";
        static const char data[] PROGMEM = "Data";
        static const char cronExpression[] PROGMEM = "CronExpression";
        static const char script[] PROGMEM = "Script";
        static const char boundDevices[] PROGMEM = "BoundDevices";
        static const char boundModules[] PROGMEM = "BoundModules";
    };
    namespace ScheduleData {
        static const char fileName[] PROGMEM = "/schedules.json";
    }

    class Schedule {
    public:
        Schedule(const char* n, const char* d, const char* dt, const char* cs, const char* jscript) {
            name = n;
            description = d;
            data = dt;
            cronExpression = cs;
            script = jscript;
        }
        ~Schedule() {
            for (auto && bd : boundDevices) {
                delete bd;
            }
            for (auto && bm : boundModules) {
                delete bm;
            }
        }
        bool check() {
            bool scheduling = false;
            cron_expr expr;
            const char* err = nullptr;
            memset(&expr, 0, sizeof(expr));
            cron_parse_expr(cronExpression.c_str(), &expr, &err);
            if (err) {
                /* invalid expression */
            } else {
                time_t now = time(nullptr) + (Config::TimeZone);
                time_t next = cron_next(&expr, now);
                scheduling = (next - now == 1);
            }
            return scheduling;
        }
        String name;
        String description;
        String data;
        String cronExpression;
        String script;
        LinkedList<String*> boundDevices; // list of device types that can use this schedule
        LinkedList<ModuleReference*> boundModules; // list of modules using this schedule
    };

    class SchedulerListener {
    public:
        virtual void onSchedule(Schedule* schedule) = 0;
    };

    class Scheduler: Task {
    public:
        Scheduler() {
            setLoopInterval(1000);
            Scheduler::load();
        }
        static void setListener(SchedulerListener* l) {
            listener = l;
        };
        // adds or updates a schedule
        static void addSchedule(Schedule*);
        static void deleteSchedule(const char* name);
        static Schedule* get(const char* name);
        static LinkedList<Schedule*> getScheduleList();
        void loop() override;
        //static void loop();
        static void load();
        static void save();
        static Schedule* getScheduleFromJson(JsonVariant json);
        static void getJson(JsonObject* jsonSchedule, Schedule* schedule);
        static String getJsonList();
    private:
        static LinkedList<Schedule*> scheduleList;
        static SchedulerListener* listener;
    };

}

#endif //HOMEGENIE_MINI_SCHEDULER_H
