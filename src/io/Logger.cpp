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
 * - 2019-01-10 Initial release
 *
 */

#include "Logger.h"

#include <net/NetManager.h>

namespace IO {

    static int logLevel;

    Logger::Logger() {
    }

    void Logger::begin(int level) {
        logLevel = level;
        // Enable serial I/O
        Serial.begin(115200);
        // TODO: not sure that this is working
        // Clear/reset the terminal screen
        // || BEGIN ANSI codes (ESC code)
        Serial.write(27); // ESC
        // clear screen command
        Serial.print("[2J");
        Serial.write(27); // ESC
        // cursor to home command
        Serial.print("[H");
        // \\ END ANSI codes
        Serial.println("\n\nBOOTING!\n");
        Log.begin(level, &Serial, false);
    }

    void Logger::loop() {

    }

    void Logger::blink() {
        // Initialize RX/TX activity LED
        //pinMode(LED_BUILTIN, OUTPUT);
        // ESP D1-mini has LED HIGH/LOW inverted
        //digitalWrite(LED_BUILTIN, HIGH);
    }

    void Logger::info(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_NOTICE) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.notice(formatted);
            Serial.println();
        }
    }

    void Logger::infoN(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_NOTICE) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.notice(formatted);
        }
    }

    void Logger::error(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_ERROR) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.error(formatted);
            Serial.println();
        }
    }

    void Logger::warn(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_WARNING) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.warning(formatted);
            Serial.println();
        }
    }

    void Logger::verbose(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_VERBOSE) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.verbose(formatted);
            Serial.println();
        }
    }

    void Logger::timestamp() {
        Serial.printf("[%s] ", Net::NetManager::getTimeClient().getFormattedDate().c_str());
    }

    void Logger::trace(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_TRACE) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.trace(formatted);
            Serial.println();
        }
    }
    void Logger::traceN(const char *s, ...) {
        if (logLevel >= LOG_LEVEL_TRACE) {
            timestamp();
            FORMAT_STRING_VARGS
            Log.trace(formatted);
        }
    }
}