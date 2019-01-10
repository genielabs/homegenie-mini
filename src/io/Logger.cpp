//
// Created by gene on 10/01/19.
//

#include "Logger.h"

namespace IO {


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
        // output timestamp
        char timestamp[15];
        snprintf(timestamp, sizeof(timestamp), "%8.06f", ((float)micros()/(float)1000000));
        Serial.printf("[%15s] ", timestamp);
        // output formatted string
        char formatted[1024];
        va_list vl;
        va_start(vl, s);
        vsnprintf(formatted, sizeof(formatted), s, vl);
        va_end(vl);
        Serial.println(String(formatted));
    }

    void Logger::log(const char *s, ...) {
        va_list arg_ptr;
        va_start(arg_ptr, s);
        info(s, arg_ptr);
        va_end(arg_ptr);
    }

    void Logger::error(const char *s, ...) {
        va_list arg_ptr;
        va_start(arg_ptr, s);
        info(s, arg_ptr);
        va_end(arg_ptr);
    }

    void Logger::warn(const char *s, ...) {
        va_list arg_ptr;
        va_start(arg_ptr, s);
        info(s, arg_ptr);
        va_end(arg_ptr);
    }
}