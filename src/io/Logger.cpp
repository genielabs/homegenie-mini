//
// Created by gene on 10/01/19.
//

#include "Logger.h"

namespace IO {

#define FORMAT_STRING_VARGS char formatted[1024]; \
                va_list vl; \
                va_start(vl, s); \
                vsnprintf(formatted, sizeof(formatted), s, vl); \
                va_end(vl);


    Logger::Logger() {
    }

    void Logger::begin() {
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
        Log.begin(LOG_LEVEL_VERBOSE, &Serial, false);
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
        timestamp();
        FORMAT_STRING_VARGS
        Log.notice(formatted);
        Serial.println();
    }

    void Logger::infoN(const char *s, ...) {
        timestamp();
        FORMAT_STRING_VARGS
        Log.notice(formatted);
    }

    void Logger::error(const char *s, ...) {
        timestamp();
        FORMAT_STRING_VARGS
        Log.error(formatted);
        Serial.println();
    }

    void Logger::warn(const char *s, ...) {
        timestamp();
        FORMAT_STRING_VARGS
        Log.warning(formatted);
        Serial.println();
    }

    void Logger::timestamp() {
        // output timestamp
        char timestamp[15];
        snprintf(timestamp, sizeof(timestamp), "%8.06f", ((float)micros()/(float)1000000));
        Serial.printf("[%15s] ", timestamp);
    }
}