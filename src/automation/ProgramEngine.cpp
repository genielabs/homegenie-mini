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

#include "ScheduledScript.h"

namespace Automation {

    using namespace IO;

    LinkedList<Schedule*> ProgramEngine::scheduleList;
    std::function<void(void*, const char* , ResponseCallback*)> ProgramEngine::apiRequest = nullptr;

    ProgramEngine::ProgramEngine() {
//        setLoopInterval(100);
    };

    void ProgramEngine::begin(std::function<void(void*, const char* , ResponseCallback*)> ar) {
        ProgramEngine::apiRequest = std::move(ar);
    }
    /*
    [[noreturn]] void ProgramEngine::worker() {
        for(;;) {
            auto jobs = &ProgramEngine::scheduleList;
            while (jobs->size() > 0) {
                ScheduledScript scheduledScript(jobs->shift());
                scheduledScript.run();
            }
            vTaskDelay(portTICK_PERIOD_MS * 100);
        }
    }
    //*/
    //*
    void ProgramEngine::loop() {
        auto jobs = &ProgramEngine::scheduleList;
        while (jobs->size() > 0) {
            ScheduledScript scheduledScript(jobs->shift());
            scheduledScript.run();
        }
    }
    //*/
    void ProgramEngine::run(Schedule* schedule) {
        ProgramEngine::scheduleList.add(schedule);
    }

}
