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
 * - 2024-01-06 Initial release
 *
 */

#include "ShutterControl.h"

namespace IO { namespace Components {

    void ShutterControl::begin() {

        shutterDriver->init();

    }

    void ShutterControl::loop() {

        long elapsed = millis() - lastCommandTs;
        float percent = ((float)elapsed / totalTimeSpanMs);
        if (stopRequested || (stopTime != 0 && millis() >= stopTime)) {

            stopRequested = false;
            stopTime = 0;
            shutterDriver->stop();
            if (lastCommand == SHUTTER_COMMAND_OPEN) {
                currentLevel += percent;
            } else if (lastCommand == SHUTTER_COMMAND_CLOSE) {
                currentLevel -= percent;
            }
            lastCommand = SHUTTER_COMMAND_NONE;
            Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
            sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);

        } else if (lastCommand == SHUTTER_COMMAND_OPEN) {

            if (elapsed > (totalTimeSpanMs - (currentLevel * totalTimeSpanMs))) {
                lastCommand = SHUTTER_COMMAND_NONE;
                shutterDriver->stop();
                currentLevel = 1;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                          &currentLevel, IOEventDataType::Float);
            } else if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                float level = currentLevel + percent;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                          &level, IOEventDataType::Float);
            }

        } else if (lastCommand == SHUTTER_COMMAND_CLOSE) {

            if (elapsed > (currentLevel * totalTimeSpanMs)) {
                lastCommand = SHUTTER_COMMAND_NONE;
                shutterDriver->stop();
                currentLevel = 0;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), currentLevel);
                sendEvent(domain.c_str(), address.c_str(), (const uint8_t*)(IOEventPaths::Status_Level), &currentLevel, IOEventDataType::Float);
            } else if (millis() - lastEventMs > EVENT_EMIT_FREQUENCY) {
                float level = currentLevel - percent;
                Logger::info("@%s [%s %.2f]", SHUTTER_CONTROL_NS_PREFIX, (IOEventPaths::Status_Level), level);
                sendEvent(domain.c_str(), address.c_str(), (const uint8_t *) (IOEventPaths::Status_Level),
                          &level, IOEventDataType::Float);
            }

        }

    }

    void ShutterControl::open() {
        if (lastCommand != SHUTTER_COMMAND_NONE) {
            stopRequested = true;
        } else {
            shutterDriver->open();
            lastCommand = SHUTTER_COMMAND_OPEN;
            lastCommandTs = millis();
        }
    }
    void ShutterControl::close() {
        if (lastCommand != SHUTTER_COMMAND_NONE) {
            stopRequested = true;
        } else {
            shutterDriver->close();
            lastCommand = SHUTTER_COMMAND_CLOSE;
            lastCommandTs = millis();
        }
    }
    void ShutterControl::setLevel(float level) {

        float levelDiff = (level / 100.f) - currentLevel;
        if (levelDiff < 0) {
            stopTime = millis() - (totalTimeSpanMs * levelDiff);
            close();
        } else if (levelDiff > 0) {
            stopTime = millis() + (totalTimeSpanMs * levelDiff);
            open();
        }
    }

    void ShutterControl::calibrate() {
        // TODO: CALIBRATE WILL set the value of "totalTimeSpanMs" variable:
        // TODO:     --> after starting "calibration" the shutter will go up
        // TODO:     --> until the user confirms with any input (e.g. the "open" button)
        // TODO:     --> that the roller/shades are fully open.
        // TODO:     --> the time that the shutter took to go from position 0 to 100% will
        // TODO:     ---> be stored to "totalTimeSpanMs" and calibration process will be complete
    }

}}
