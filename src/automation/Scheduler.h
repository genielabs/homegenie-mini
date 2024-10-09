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

#include <LinkedList.h>
#include <LittleFS.h>

#include "data/Module.h"
#include "ExtendedCron.h"

#define SCHEDULER_NS_PREFIX            "Automation::Scheduler"

namespace Automation {

    using namespace Data;

    namespace ScheduleField {
        static const char name[] = "Name";
        static const char description[] = "Description";
        static const char data[] = "Data";
        static const char cronExpression[] = "CronExpression";
        static const char script[] = "Script";
        static const char boundDevices[] = "BoundDevices";
        static const char boundModules[] = "BoundModules";
    };
    namespace ScheduleData {
        static const char fileName[] = "/schedules.json";
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
        bool occurs(time_t ts) {
            return ExtendedCron::IsScheduling(ts, cronExpression);
        }
        bool wasScheduled(time_t ts) const {
            return ExtendedCron::normalizeStartTime(lastOccurrence) == ExtendedCron::normalizeStartTime(ts);
        }
        void setScheduled(time_t ts) {
            lastOccurrence = ts;
        }
        bool isEnabled = true;
        String name;
        String description;
        String data;
        String cronExpression;
        String script;
        LinkedList<String*> boundDevices; // list of device types that can use this schedule
        LinkedList<ModuleReference*> boundModules; // list of modules using this schedule
    private:
        time_t lastOccurrence;
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
        // TODO: add LinkedList<RuntimeEvent*> runtimeEventList;  ( RuntimeEvent => {key/value} )
        //       - registerRuntimeEvent(const char* key, const char* description, ..)
        //       - handleRuntimeEvent(const char* key, const char* value, ..)
        static SchedulerListener* listener;
        static int lastCheckMinute;
    };

}

#endif //HOMEGENIE_MINI_SCHEDULER_H
