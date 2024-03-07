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
 * - 2019-01-10 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_LOGGER_H
#define HOMEGENIE_MINI_LOGGER_H

#include <ArduinoLog.h>

#define FORMAT_STRING_VARGS char formatted[1024]; \
                va_list vl; \
                va_start(vl, s); \
                vsnprintf(formatted, sizeof(formatted), s, vl); \
                va_end(vl);
/*
#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_NOTICE  4
#define LOG_LEVEL_TRACE   5
#define LOG_LEVEL_VERBOSE 6
 */

namespace IO {

    class Logger {
    public:
        static void begin(int level);

        static void error(const char *s, ...);
        static void warn(const char *s, ...);
        static void info(const char *s, ...);
        static void infoN(const char *s, ...);
        static void trace(const char *s, ...);
        static void traceN(const char *s, ...);
        static void verbose(const char *s, ...);

    private:
        Logger();
        static void timestamp();
    };

}

#endif //HOMEGENIE_MINI_LOGGER_H
