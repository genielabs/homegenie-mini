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
 */

#ifndef HOMEGENIE_MINI_STATISTICS_H
#define HOMEGENIE_MINI_STATISTICS_H

#include "io/IOEventData.h"
#include "Module.h"

#ifndef DISABLE_DATA_PROCESSING

namespace Data { namespace Processing {

    using IO::IOEventDataType;
    using Data::ModuleParameter;

    class StatValue {
    public:
        ModuleParameter *parameter;
        double value;
        double timestamp;

        StatValue() {
            parameter = nullptr;
            value = 0;
            timestamp = 0;
        }
    };

    const unsigned int STATS_HISTORY_LIMIT = 2000;
    const unsigned int STATS_HISTORY_RESULTS_DEFAULT_PAGE_SIZE = 100;

    class Statistics {
    public:
        Statistics() {
            History = (StatValue **) ps_malloc(STATS_HISTORY_LIMIT * sizeof(StatValue *));
        }

        static size_t getHistorySize() {
            return historyIsFull ? STATS_HISTORY_LIMIT : historyIndex;
        }

        static StatValue *getHistory(int position) {
            auto i = (historyIndex - position - 1);
            if (i < 0) i = STATS_HISTORY_LIMIT - i;
            return History[i % STATS_HISTORY_LIMIT];
        }

        static unsigned int collect(ModuleParameter *parameter) {
            auto ci = historyIndex;
            auto sv = (StatValue *) ps_malloc(sizeof(StatValue));
            sv->parameter = parameter;
            sv->value = parameter->value.toDouble();
            // unix timestamp in milliseconds
            sv->timestamp = ((double) Config::getRTC()->getEpoch()) + ((double) (millis() % 1000) / 1000.0f);

            if (historyIsFull) free(History[ci]);
            History[ci] = sv;

            // circular / overwrite old data...
            if (++historyIndex > STATS_HISTORY_LIMIT - 1) {
                historyIndex = 0;
                historyIsFull = true;
            }

            return ci;
        }

    private:
        static int historyIndex;
        static StatValue **History;
        static bool historyIsFull;
    };

}}

#endif // DISABLE_DATA_PROCESSING

#endif //HOMEGENIE_MINI_STATISTICS_H
