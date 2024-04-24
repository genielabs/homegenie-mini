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

#include "ExtendedCron.h"
#include "Scheduler.h"

#include "Utility.h"

namespace Automation {

    Schedule* getCustomEvent(const char* eventName) {
        // TODO: to be implemented
        return nullptr;
    }

    bool ExtendedCron::IsScheduling(time_t date, String& cronExpression, int recursionCount)
    {
        return getScheduling(date, date, cronExpression, recursionCount).size() > 0;
    }

    time_t ExtendedCron::normalizeStartTime(time_t timestamp) {
        // TODO: dateStart.AddSeconds(-dateStart.Second).AddMilliseconds(-dateStart.Millisecond)
        return timestamp - (timestamp % 60);
    }
    time_t ExtendedCron::normalizeEndTime(time_t timestamp) {
        // TODO: dateEnd.AddSeconds(-dateEnd.Second).AddMilliseconds(-dateEnd.Millisecond)
        //                .AddSeconds(59).AddMilliseconds(999)
        return normalizeStartTime(timestamp) + 59;
    }

    LinkedList<time_t> ExtendedCron::getScheduling(time_t dateStart, time_t dateEnd, String& cronExpression, int recursionCount)
    {
        // align input time
        if (!hasSecondsField(cronExpression.c_str())) {
            dateStart = normalizeStartTime(dateStart);
            dateEnd = normalizeEndTime(dateEnd);
        }

        // '[' and ']' are just aesthetic alias for '(' and ')'
        cronExpression.replace("[", "(");
        cronExpression.replace("]", ")");

        String specialChars = "() ;&:|>%!";
        unsigned int charIndex = 0;
        auto rootEvalNode = new EvalNode();
        auto evalNode = rootEvalNode;
        while (charIndex < cronExpression.length())
        {
            char token = cronExpression.charAt(charIndex);
            if (token == '\t' || token == '\r' || token == '\n')
                token = ' ';
            if (specialChars.indexOf(token) >= 0)
            {
                switch (token)
                {
                    case '(':
                        evalNode->Child = new EvalNode(evalNode);
                        evalNode = (EvalNode*)evalNode->Child;
                        break;

                    case ')':
                        if (evalNode->Parent != nullptr)
                        {
                            evalNode = (EvalNode*)evalNode->Parent;
                        }
                        else
                        {
                            //masterControlProgram.HomeGenie.MigService.RaiseEvent(this,
                            //    Domains.HomeAutomation_HomeGenie,
                            //    SourceModule.Scheduler, cronExpression, Properties.SchedulerError,
                            //    JsonConvert.SerializeObject("Unbalanced parenthesis in '" + cronExpression + "'"));
                            // TODO: throw error?  -->  "Unbalanced parenthesis in '" + cronExpression + "'"
                            return {};
                        }
                        break;

                    case ';': // AND
                    case '&': // AND
                    case ':': // OR
                    case '|': // OR
                    case '>': // TO
                    case '%': // NOT
                    case '!': // NOT
                        // collect operator and switch to next node
                        evalNode->Operator = token;
                        evalNode->Sibling = new EvalNode((EvalNode*)evalNode->Parent);
                        evalNode = (EvalNode*)evalNode->Sibling;
                        break;
                }

                charIndex++;
                continue;
            }

            String currentExpression = String(token);
            charIndex++;
            while (charIndex < cronExpression.length()) // collecting plain cron expression
            {
                token = cronExpression[charIndex];
                if (token != ' ' && specialChars.indexOf(token) >= 0)
                {
                    break;
                }

                currentExpression += token;
                charIndex++;
            }

            currentExpression.trim();
            if (currentExpression.isEmpty())
                continue;

            evalNode->Expression = currentExpression;

            if (currentExpression.startsWith("#"))
            {
                // TODO: ...?
            }
            else if (currentExpression.startsWith("@"))
            {
                // TODO: example
                //   @SolarTimes.Sunset + 30
                auto start = dateStart;
                int addMinutes = 0;
                if (currentExpression.indexOf('+') > 0)
                {
                    auto addMin = currentExpression.substring(currentExpression.lastIndexOf('+'));
                    removeWhiteSpaces(addMin);
                    addMinutes = addMin.toInt();
                    currentExpression = currentExpression.substring(0, currentExpression.lastIndexOf('+'));
                }
                else if (currentExpression.indexOf('-') > 0)
                {
                    auto addMin = currentExpression.substring(currentExpression.lastIndexOf('-'));
                    removeWhiteSpaces(addMin);
                    addMinutes = addMin.toInt();
                    currentExpression = currentExpression.substring(0, currentExpression.lastIndexOf('-'));
                }
                auto eventName = currentExpression.substring(1);
                removeWhiteSpaces(eventName);
                if (eventName == "SolarTimes.Sunrise") {
                    // TODO: handleSunrise(evalNode, start, dateEnd, addMinutes);
                } else
                if (eventName == "SolarTimes.Sunset") {
                    // TODO: handleSunset(evalNode, start, dateEnd, addMinutes);
                } else
                if (eventName == "SolarTimes.SolarNoon") {
                    // TODO: handleSolarNoon(evalNode, start, dateEnd, addMinutes);
                } else {
                    // TODO: lookup for user-defined @ events
                        // Check expression from scheduled item with a given name
                        auto eventItem = getCustomEvent(eventName.c_str());
                        if (eventItem == nullptr)
                        {
                            // TODO: signal error -->  "Unknown event name '" + currentExpression + "'"
                        }
                        else if (recursionCount >= EXTENDED_CRON_MAX_EVAL_RECURSION)
                        {
                            recursionCount = 0;
                            // TODO: signal error --> "Too much recursion in expression '" + currentExpression + "'"
                            eventItem->isEnabled = false;
                        }
                        else
                        {
                            recursionCount++;
                            if (eventItem->isEnabled)
                            {
                                evalNode->Occurrences = getScheduling(dateStart - (addMinutes * 60),
                                                                     dateEnd - (addMinutes * 60), eventItem->cronExpression, recursionCount);
                                if (addMinutes != 0)
                                {
                                    for (short o = 0; o < evalNode->Occurrences.size(); o++)
                                    {
                                        evalNode->Occurrences.set(o, evalNode->Occurrences.get(o) + (addMinutes * 60));
                                    }
                                }
                            }

                            recursionCount--;
                            if (recursionCount < 0)
                                recursionCount = 0;
                        }
                }
            }
            else
            {
                getNextOccurrences(evalNode->Occurrences, dateStart, dateEnd, currentExpression);
            }
        }

        auto result = evalNodes(rootEvalNode);
        auto copy = LinkedList<time_t>();
        for (int i = 0; i < result->size(); i++) {
            copy.add(result->get(i));
        }
        delete rootEvalNode;
        return copy;
    }

    int ExtendedCron::hasSecondsField(const char* str) {
        char del = ' ';
        size_t count = 0;
        if (!str) return -1;
        while ((str = strchr(str, del)) != NULL) {
            count++;
            do str++; while (del == *str);
        }
        return (int)count + 1 > 5;
    }

    void ExtendedCron::getNextOccurrences(LinkedList<time_t>& occurrences, time_t dateStart, time_t dateEnd, const String& cronExpression)
    {

        for (time_t ts = dateStart - 1; ts < dateEnd; ts += 60) {
            cron_expr expr;
            const char *err = nullptr;
            memset(&expr, 0, sizeof(expr));
            cron_parse_expr(cronExpression.c_str(), &expr, &err);
            if (err) {
                /* invalid expression */
            } else {
                time_t checkTime = ts; // + (Config::TimeZone);
                time_t next = cron_next(&expr, checkTime);
                if (next - checkTime == 1) {
                    occurrences.add(ts + 1);
                }
            }
        }

    }

    LinkedList<time_t>* ExtendedCron::evalNodes(EvalNode* currentNode)
    {
        auto occurs = &currentNode->Occurrences;
        if (currentNode->Child != nullptr)
        {
            occurs = evalNodes((EvalNode*)currentNode->Child);
        }
        if (currentNode->Sibling != nullptr && currentNode->Operator != ' ' && currentNode->Operator != '\0')
        {
            auto matchList = evalNodes((EvalNode*)currentNode->Sibling);
            if (currentNode->Operator == ':' || currentNode->Operator == '|')
            {
                // Include occurrences (OR)
                for (auto o : *matchList)
                    occurs->add(o);
            }
            else if (currentNode->Operator == '%' || currentNode->Operator == '!')
            {
                // Exclude occurrences (NOT)
                for (auto o : *matchList) {
                    for (int i = 0; i < occurs->size(); i++) {
                        auto oo = occurs->get(i);
                        if (oo == o) {
                            occurs->remove(i);
                            break;
                        }
                    }
                }
            }
            else if (currentNode->Operator == ';' || currentNode->Operator == '&')
            {
                // Intersect occurrences (AND)
                for (int i = occurs->size() - 1; i >= 0; i--) {
                    auto oo = occurs->get(i);
                    bool exists = false;
                    for (auto o : *matchList) {
                        if (oo == o) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) occurs->remove(i);
                }
            }
            else if (currentNode->Operator == '>')
            {
                if (matchList->size() > 0 && occurs->size() > 0)
                {
                    time_t start = occurs->get(occurs->size() - 1); // last
                    time_t end = matchList->get(0); // first
                    time_t inc = start + 60;
                    while (end != inc && start != inc)
                    {
                        occurs->add(inc);
                        struct tm *tm_struct = localtime(&inc);
                        if (tm_struct->tm_hour == 23 && tm_struct->tm_min == 59)
                        {
                            inc = inc - (23 * 60 * 60); // 23hrs
                            inc = inc - (59 * 60); // 59min
                        }
                        else
                        {
                            inc = inc + 60; // move to next minute
                        }
                    }
                    for (auto o: *matchList) {
                        occurs->add(o);
                    }
                }
            }
        }
        return occurs;
    }

    void ExtendedCron::removeWhiteSpaces(String &s) {
        s.replace(" ", "");
        s.replace("\t", "");
        s.replace("\v", "");
        s.replace("\f", "");
        s.replace("\r", "");
        s.replace("\n", "");
    }

}