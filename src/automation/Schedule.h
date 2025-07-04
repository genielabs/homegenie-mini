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

#ifndef HOMEGENIE_MINI_SCHEDULE_H
#define HOMEGENIE_MINI_SCHEDULE_H

#include "data/Module.h"
#include "ExtendedCron.h"
#include "service/EventRouter.h"

namespace Automation {

    using namespace Data;
    using namespace Service;

    namespace ScheduleField {
        static const char name[] = "Name";
        static const char description[] = "Description";
        static const char data[] = "Data";
        static const char cronExpression[] = "CronExpression";
        static const char script[] = "Script";
        static const char boundDevices[] = "BoundDevices";
        static const char boundModules[] = "BoundModules";
        static const char eventModules[] = "EventModules";
        static const char onModuleEvent[] = "OnModuleEvent";
        static const char isEnabled[] = "IsEnabled";
    };
    namespace ScheduleData {
        static const char fileName[] = "/schedules.json";
    }

    class ScheduleInfo {
    public:
        time_t onPreviousMinute = -1;
        time_t onNextMinute = -1;
        time_t onThisMinute = -1;
        time_t lastProcessedMinute = -1;
    };

    class Schedule {
    public:
        Schedule(const char *n, const char *d, const char *dt, const char *cs, const char *jscript) {
            name = n;
            description = d;
            data = dt;
            cronExpression = cs;
            script = jscript;
        }

        ~Schedule() {
            for (auto &&bd: boundDevices) {
                delete bd;
            }
            for (auto &&bm: boundModules) {
                delete bm;
            }
            for (auto &&em: eventModules) {
                delete em;
            }
        }

        bool occursUpdate(time_t ts) {
            const uint8_t oneMinuteInSeconds = 60;
            time_t now = ExtendedCron::normalizeStartTime(ts);

            if (now <= RTC_SYNC_THRESHOLD) {
                if (info.lastProcessedMinute != now) {
                    info.onPreviousMinute = 0;
                    info.onThisMinute = 0;
                    info.onNextMinute = 0;
                    info.lastProcessedMinute = now;
                }
                return false;
            }

            if (info.lastProcessedMinute == now) {
                return info.onThisMinute != 0;
            }

            unsigned long st = millis();
            bool canSlide = (info.lastProcessedMinute != -1) &&
                            (now == info.lastProcessedMinute + oneMinuteInSeconds);
            if (canSlide) {
                info.onPreviousMinute = info.onThisMinute;
                info.onThisMinute = info.onNextMinute;
                info.onNextMinute = occurs(now + oneMinuteInSeconds) ? (now + oneMinuteInSeconds) : 0;
            } else {
                info.onPreviousMinute = occurs(now - oneMinuteInSeconds) ? (now - oneMinuteInSeconds) : 0;
                info.onThisMinute = occurs(now) ? now : 0;
                info.onNextMinute = occurs(now + oneMinuteInSeconds) ? (now + oneMinuteInSeconds) : 0;
            }

            info.lastProcessedMinute = now;
            return info.onThisMinute != 0;
        }

        bool occurs(time_t ts) {
            return ExtendedCron::IsScheduling(ts, cronExpression);
        }

        bool wasScheduled(time_t ts) const {
            ts = ExtendedCron::normalizeStartTime(ts);
            return lastOccurrence == ts;
        }

        void setScheduled(time_t ts) {
            ts = ExtendedCron::normalizeStartTime(ts);
            lastOccurrence = ts;
        }

        std::shared_ptr<QueuedMessage> getLastEvent() {
            return lastEvent;
        }
        void setLastEvent(const std::shared_ptr<QueuedMessage>& m) {
            lastEvent = nullptr;
            if (m) {
                auto qm = std::make_shared<QueuedMessage>();
                qm->type = m->type;
                qm->domain = m->domain;
                qm->sender = m->sender;
                qm->event = m->event;
                qm->value = m->value;
                // TODO: create a deep-copy mechanism to handle data
                qm->data = nullptr; //m->data; can't be just assigned because "dispose" might not be directly handled here
                lastEvent = qm;
            }
        }

        bool isEnabled = true;
        bool triggered = false;
        bool onModuleEvent = false;
        String name;
        String description;
        String data;
        String cronExpression;
        String script;
        ScheduleInfo info;
        LinkedList<String *> boundDevices; // list of device types that can use this schedule
        LinkedList<ModuleReference*> boundModules; // list of modules using this schedule
        LinkedList<ModuleReference*> eventModules; // list of modules that will start this schedule when the value of a parameter of these modules changes
        static const long RTC_SYNC_THRESHOLD = 645120000;

    private:
        time_t lastOccurrence{};
        std::shared_ptr<QueuedMessage> lastEvent;

    };
}

#endif //HOMEGENIE_MINI_SCHEDULE_H
