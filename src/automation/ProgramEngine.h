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

#ifndef HOMEGENIE_MINI_PROGRAMENGINE_H
#define HOMEGENIE_MINI_PROGRAMENGINE_H

#include "Task.h"

#include "automation/Scheduler.h"
#include "io/Logger.h"
#include "net/NetManager.h"

#define PROGRAMENGINE_NS_PREFIX            "Automation::ProgramEngine"

namespace Automation {

    using namespace Net;

    class ProgramEngine
#ifndef CONFIG_CREATE_AUTOMATION_TASK
            : Task
#endif
            {
    public:
        ProgramEngine();
        static void begin(std::function<void(void*, const char* , ResponseCallback*)>);

#ifdef CONFIG_CREATE_AUTOMATION_TASK
        [[noreturn]] static void worker();
#else
        void loop() override;
#endif
        static void run(Schedule*);
        static std::function<void(void*, const char* , ResponseCallback*)> apiRequest;
    private:
        static LinkedList<Schedule*> scheduleList;
    };

}

#endif //HOMEGENIE_MINI_PROGRAMENGINE_H
