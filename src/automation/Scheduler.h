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

#include "Schedule.h"

#define SCHEDULER_NS_PREFIX            "Automation::Scheduler"

namespace Automation {

    class SchedulerListener {
    public:
        virtual void onSchedule(Schedule* schedule) = 0;
    };

#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
    class Scheduler
#else
    class Scheduler: Task
#endif
    {
    public:
        Scheduler() {
#ifndef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
            setLoopInterval(1000);
#endif
            Scheduler::load();
        }
        static void setListener(SchedulerListener* l) {
            listener = l;
        };
        // adds or updates a schedule
        static void addSchedule(Schedule*);
        static void enableSchedule(const char* name);
        static void disableSchedule(const char* name);
        static void deleteSchedule(const char* name);
        static Schedule* get(const char* name);
        static LinkedList<Schedule*> getScheduleList();
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
        static void worker();
#else
        void loop() override;
#endif
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
        static ModuleReference* parseModuleReference(JsonObject& em);
    };

}

#endif //HOMEGENIE_MINI_SCHEDULER_H
