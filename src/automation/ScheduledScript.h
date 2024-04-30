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

#ifndef HOMEGENIE_MINI_SCHEDULEDSCRIPT_H
#define HOMEGENIE_MINI_SCHEDULEDSCRIPT_H

#include "HomeGenie.h"

#ifndef DISABLE_AUTOMATION

#include <lib/duktape-2.7.0/src/duktape.h>

/*
 *

Without Duktape

RAM:   [==        ]  19.2% (used 63060 bytes from 327680 bytes)
Flash: [========  ]  83.7% (used 1645138 bytes from 1966080 bytes)

With Duktape +200k of flash

RAM:   [==        ]  19.2% (used 63068 bytes from 327680 bytes)
Flash: [========= ]  93.8% (used 1843314 bytes from 1966080 bytes)

With Duktape (optimized build) +165k of flash

RAM:   [==        ]  19.3% (used 63100 bytes from 327680 bytes)
Flash: [========= ]  92.1% (used 1810372 bytes from 1966080 bytes)

// TODO: add additional project-wise optimizations to save more Flash

 *
 */

#include "automation/helpers/NetHelper.h"
#include "automation/ProgramEngine.h"

namespace Automation {

    using namespace Net;
    using namespace Service;
    using namespace Service::API;

    class ScheduledScript {
    public:
        ScheduledScript() = default;
        void setSchedule(Schedule* s) {
            schedule = s;
        }
        explicit ScheduledScript(Schedule*);
        void run();
    private:
        Schedule* schedule;

        static duk_ret_t helper_log(duk_context *ctx);
        static duk_ret_t pause(duk_context *ctx);

        static duk_ret_t schedule_on_previous(duk_context *ctx);
        static duk_ret_t schedule_on_next(duk_context *ctx);

        static duk_ret_t boundModules_command(duk_context *ctx);
        static duk_ret_t boundModules_property_get(duk_context *ctx);
        static duk_ret_t boundModules_property_avg(duk_context *ctx);

        static duk_ret_t netHelper_call(duk_context *ctx);
        static duk_ret_t netHelper_ping(duk_context *ctx);

        static const char* getProperty(duk_context *ctx, const char* propertyPath);
        static float getAvgPropertyValue(duk_context *ctx, const char* propertyPath);

        static void apiCommand(duk_context *ctx, const char* command, const char* options);

    };
};


#endif //HOMEGENIE_MINI_SCHEDULEDSCRIPT_H

#endif