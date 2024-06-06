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

namespace Automation {

    Schedule* getCustomEvent(const char* eventName) {
        // TODO: to be implemented
        return nullptr;
    }

    bool ExtendedCron::IsScheduling(time_t date, String& cronExpression)
    {
        struct tm timeStart;
        localtime_r(&date, &timeStart);
        timeStart.tm_sec = timeStart.tm_min = timeStart.tm_hour = 0;
        struct tm timeEnd;
        localtime_r(&date, &timeEnd);
        timeEnd.tm_sec = 59;
        timeEnd.tm_min = 59;
        timeEnd.tm_hour = 23;
        auto start = mktime(&timeStart);
        auto end = mktime(&timeEnd);
        auto list = getScheduling(start, end, cronExpression);
        int low = 0;
        int high = list.size() - 1;
        time_t x = normalizeStartTime(date);
        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (list[mid] == x) {
                return true;
            }
            if (list[mid] < x) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return false;
    }

    time_t ExtendedCron::normalizeStartTime(time_t timestamp) {
        return timestamp - (timestamp % 60);
    }
    time_t ExtendedCron::normalizeEndTime(time_t timestamp) {
        return normalizeStartTime(timestamp) + 59;
    }

    LinkedList<time_t> ExtendedCron::getScheduling(time_t dateStart, time_t dateEnd, String& cronExpression, int recursionCount)
    {
        // align input time
        dateStart = normalizeStartTime(dateStart);
        dateEnd = normalizeEndTime(dateEnd);

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
                // example:
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
                    handleSunrise(evalNode, start, dateEnd, addMinutes);
                } else
                if (eventName == "SolarTimes.Sunset") {
                    handleSunset(evalNode, start, dateEnd, addMinutes);
                } else
                if (eventName == "SolarTimes.SolarNoon") {
                    handleSolarNoon(evalNode, start, dateEnd, addMinutes);
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
        copy.sort(occurrencesCompare);
        return copy;
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

    void ExtendedCron::handleSunrise(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes) {
        auto sd = gmtime(&dateStart);
        // Calculate the times of sunrise, transit, and sunset, in hours (UTC) (relative from midnight of the selected day)
        double relTransit, relSunrise, relSunset;
        calcSunriseSunset(sd->tm_year + 1900, sd->tm_mon + 1, sd->tm_mday, Config::zone.latitude, Config::zone.longitude, relTransit, relSunrise, relSunset);
        // Translate to local time
        time_t sunrise = Utility::relativeUtcHoursToLocalTime(relSunrise, dateStart) + (addMinutes * 60);
        if (sunrise >= dateStart && sunrise <= dateEnd) {
            (&evalNode->Occurrences)->add(sunrise);
        }
    }

    void ExtendedCron::handleSunset(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes) {
        auto sd = gmtime(&dateStart);
        // Calculate the times of sunrise, transit, and sunset, in hours (UTC) (relative from midnight of the selected day)
        double relTransit, relSunrise, relSunset;
        calcSunriseSunset(sd->tm_year + 1900, sd->tm_mon + 1, sd->tm_mday, Config::zone.latitude, Config::zone.longitude, relTransit, relSunrise, relSunset);
        // Translate to local time
        time_t sunset = Utility::relativeUtcHoursToLocalTime(relSunset, dateStart) + (addMinutes * 60);
        if (sunset >= dateStart && sunset <= dateEnd) {
            (&evalNode->Occurrences)->add(sunset);
        }
    }

    void ExtendedCron::handleSolarNoon(EvalNode* evalNode, time_t dateStart, time_t dateEnd, time_t addMinutes) {
        auto sd = gmtime(&dateStart);
        // Calculate the times of sunrise, transit, and sunset, in hours (UTC) (relative from midnight of the selected day)
        double relTransit, relSunrise, relSunset;
        calcSunriseSunset(sd->tm_year + 1900, sd->tm_mon + 1, sd->tm_mday, Config::zone.latitude, Config::zone.longitude, relTransit, relSunrise, relSunset);
        // Translate to local time
        time_t solarNoon = Utility::relativeUtcHoursToLocalTime(relTransit, dateStart) + (addMinutes * 60);
        if (solarNoon >= dateStart && solarNoon <= dateEnd) {
            (&evalNode->Occurrences)->add(solarNoon);
        }
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