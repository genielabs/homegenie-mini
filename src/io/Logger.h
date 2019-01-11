//
// Created by gene on 10/01/19.
//

#ifndef HOMEGENIE_MINI_LOGGER_H
#define HOMEGENIE_MINI_LOGGER_H

#include <Arduino.h>
#include <ArduinoLog.h>

namespace IO {

    enum LogLevel {
        Info,
        Warn,
        Error,
        Debug,
        Trace
    };

    class Logger {
    public:
        static void begin();
        static void loop();
        static void info(const char *s, ...);
        static void infoN(const char *s, ...);
        static void error(const char *s, ...);
        static void warn(const char *s, ...);
        static void log(const char *s, ...);
        static void blink();
    private:
        Logger();
        static void timestamp();
    };

}

#endif //HOMEGENIE_MINI_LOGGER_H
