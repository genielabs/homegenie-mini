/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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

#include "TaskManager.h"

static Task *taskList = NULL, *currentTask = NULL;

TaskManager::TaskManager() {
    taskList = NULL;
    currentTask = NULL;
}

void TaskManager::loop() {
    // TODO: (maybe) implement a simple `Priority` mechanism
    // TODO: (maybe) implement a "loop() Load" index by measuring time elapsed till end of `loop()` method
    IO::Logger::verbose("%s loop() >> BEGIN", TASKMANAGER_LOG_PREFIX);
    Task *t = taskList;
    uint c = 0;
    while (t != NULL) {
        IO::Logger::verbose("%s - running task %d", TASKMANAGER_LOG_PREFIX, c++);
        if (t->willLoop()) {
            t->loop();
            t->loopExit();
        }
        t = t->nextTask;
    }
    IO::Logger::verbose("%s loop() << END", TASKMANAGER_LOG_PREFIX);
}

void TaskManager::addTask(Task *task) {
    if (taskList == NULL) {
        taskList = currentTask = task;
        taskList->nextTask = NULL;
        taskList->previousTask = NULL;
    } else {
        currentTask->nextTask = task;
        task->previousTask = currentTask;
        task->nextTask = NULL;
        currentTask = task;
    }
}

// TODO: implement task disposal as well (eg. removeTask(...))
