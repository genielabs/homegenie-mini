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
 * - 2024-04-20 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_EXTENDEDCRON_H
#define HOMEGENIE_MINI_EXTENDEDCRON_H

#include <LinkedList.h>
#include <SolarCalculator.h>

#include "lib/supertinycron/ccronexpr.h"

#include "Config.h"
#include "Utility.h"

#define EXTENDED_CRON_MAX_EVAL_RECURSION 4

namespace Automation {

    class EvalNode
    {
    public:
        LinkedList<time_t> Occurrences;
        void* Child = nullptr; // EvalNode
        void* Parent = nullptr; // EvalNode
        void* Sibling = nullptr; // EvalNode
        String Expression;
        char Operator;

        EvalNode() = default;
        explicit EvalNode(EvalNode* parentNode): EvalNode() {
            Parent = parentNode;
        }

        ~EvalNode() {
            Occurrences.clear();
            delete (EvalNode*)Child;
            delete (EvalNode*)Sibling;
        }
    };

    class ExtendedCron {
    public:
        static bool IsScheduling(time_t date, String& cronExpression);

        static time_t normalizeStartTime(time_t timestamp);
        static time_t normalizeEndTime(time_t timestamp);

        static LinkedList<time_t> getScheduling(time_t dateStart, time_t dateEnd, String& cronExpression, int recursionCount = 0);

        static int hasSecondsField(const char* str);

    private:
        static LinkedList<time_t>* evalNodes(EvalNode* currentNode);
        static void getNextOccurrences(LinkedList<time_t>& occurrences, time_t dateStart, time_t dateEnd, const String& cronExpression);
        static void removeWhiteSpaces(String& s);
        static void handleSunrise(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes);
        static void handleSunset(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes);
        static void handleSolarNoon(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes);
        static int occurrencesCompare(time_t& a, time_t& b) {
            return a == b ? 0 : a > b ? 1 : -1;
        }
    };

}


#endif //HOMEGENIE_MINI_EXTENDEDCRON_H
