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

#include "ProgramEngine.h"

#ifndef DISABLE_AUTOMATION

#include "ScheduledScript.h"

namespace Automation {

    using namespace IO;

    LinkedList<ScheduleRef> ProgramEngine::scheduleList;
    std::function<void(void*, const char* , ResponseCallback*)> ProgramEngine::apiRequest = nullptr;

    ProgramEngine::ProgramEngine() {
#ifndef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
        setLoopInterval(100);
#endif
    };

    void ProgramEngine::begin(std::function<void(void*, const char* , ResponseCallback*)> ar) {
        ProgramEngine::apiRequest = std::move(ar);
    }

    void ProgramEngine::runNext() {
        ScheduleRef next;
        {
            SchedulerLock lock;
            if (scheduleList.size()) next = scheduleList.shift();
        }
        if (!next) return;
        ScheduledScript(next.get()).run();
        if (next->oneTime) next->oneTimeFinished = true;
    }

#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
    [[noreturn]] void ProgramEngine::worker() {
        for(;;) {
            runNext();
            vTaskDelay(100 * portTICK_PERIOD_MS);
        }
    }
#else
    bool isRunning;
    void ProgramEngine::loop() {
        if (isRunning) return;
        isRunning = true;
        runNext();
        isRunning = false;
    }
#endif

    void ProgramEngine::run(Schedule* schedule) {
        auto current = Scheduler::find(schedule->name.c_str());
        if (current.get() == schedule) run(current);
    }

    void ProgramEngine::run(ScheduleRef schedule) {
        SchedulerLock lock;
        bool exists = false;
        for (auto s : scheduleList) {
            if (s == schedule) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            ProgramEngine::scheduleList.add(schedule);
        }
    }

}

#endif
