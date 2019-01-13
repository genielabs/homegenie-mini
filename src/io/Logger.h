//
// Created by gene on 10/01/19.
//

#ifndef HOMEGENIE_MINI_LOGGER_H
#define HOMEGENIE_MINI_LOGGER_H

#include <Arduino.h>
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
        static void loop();

        static void error(const char *s, ...);
        static void warn(const char *s, ...);
        static void info(const char *s, ...);
        static void infoN(const char *s, ...);
        static void trace(const char *s, ...);
        static void verbose(const char *s, ...);
        static void blink();

    private:
        Logger();
        static void timestamp();
    };

}

#endif //HOMEGENIE_MINI_LOGGER_H
