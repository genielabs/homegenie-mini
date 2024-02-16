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

#ifndef HOMEGENIE_MINI_TASK_H
#define HOMEGENIE_MINI_TASK_H

#include "Config.h"

// Task interface

class Task {
public:
    Task();
    /**
     * Task loop() entry point. This is called by the TaskManager when the task is scheduled.
     */
    virtual void loop() = 0;
    /**
     * If the task loop() is scheduled.
     * @return `true` if scheduled, `false` otherwise.
     */
    virtual bool willLoop();
    /**
     * Set task loop() schedule interval.
     * @param interval_ms schedule interval in milliseconds.
     */
    void setLoopInterval(uint64_t interval_ms) { loopInterval = interval_ms; };

    /// Pointer to the next task
    Task* nextTask = nullptr;
    /// Pointer to the previous task
    Task* previousTask = nullptr;

    void loopExit();
    uint64_t taskIdleTime() const;
    uint64_t uptime() {
        return millis() - creationTs;
    }

private:
    uint64_t creationTs;
    uint64_t loopInterval = 0;
protected:
    uint64_t lastLoopTs = 0;
};


#endif //HOMEGENIE_MINI_TASK_H
