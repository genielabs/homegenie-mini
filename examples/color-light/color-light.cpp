/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
 */

#include "color-light.h"

void setup() {
    // Default name shown in SNMP/UPnP advertising
    Config::system.friendlyName = "LED Controller";

    // Get a reference to HomeGenie Mini running instance
    homeGenie = HomeGenie::getInstance();

    // Get the default system module
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->name = "LED Controller";

    // Read stored config
    ledsPin = (int16_t)Config::getSetting(Configuration::LedsPin).toInt();
    pixelsType = (int16_t)Config::getSetting(Configuration::LedsType).toInt();
    pixelsSpeed = (int16_t)Config::getSetting(Configuration::LedsSpeed).toInt();
    // the following settings can also be configured from the app
    ledsCount = abs(Config::getSetting(Configuration::LedsCount).toInt());
    maxPower = Config::getSetting(Configuration::LedsPower).toInt();
    if (maxPower <= 0) maxPower = DEFAULT_MAX_POWER;

    // Add UI options to configure this device using the app
    setupControllerOptions(miniModule);

    // Setup builtin RGB status led
    statusLed.setup();
    colorLight = statusLed.getColorLight();

    isConfigured = Config::isDeviceConfigured();
    if (!isConfigured) {

        Config::setOnWiFiConfiguredCallback([]{
            // Device successfully configured
            Serial.println("Restarting now.");
            delay(2000);
            ESP.restart();
            // TODO: re-arrange code so that it won't require a restart
        });

    } else {

        createPixels();

        // colorLight is the main control module
        colorLight->onSetColor([](LightColor color) {
            statusLed.setCurrentColor(color);
            fx_reset(pixels, color);
        });

        auto module = colorLight->module;
        module->name = "Smart Light";

        // Add ColorLight UI options for setting
        // animation style and strobe effects.
        setupColorLightOptions(module);

        // Setup physical control buttons
        setupControlButtons(module);

#ifndef DISABLE_AUTOMATION
        // Setup example schedules/scenes
        setupDefaultSchedules();
#endif

        // Initialize FX buffer
        fx_init(ledsCount, statusLed.getCurrentColor());

        // TODO: implement color/status recall on start
        // Set default color
        float h = presetColors[0][0];
        float s = presetColors[0][1];
        colorLight->setColor(h, s, 0, 0);

    }

    homeGenie->begin();

    refresh();
}

void loop()
{
    homeGenie->loop();

    if (isConfigured) {

        if (millis() - lastRefreshTs > refreshMs)
        {

            if (!strobeOff && strobeFxTickMs > 0 && millis() - strobeFxTickMs > ((strobeFxDurationMs + strobeFxIntervalMs) * (mpFxStrobe->value == "slow" ? 3 : mpFxStrobe->value == "medium" ? 2 : 1))) {

                // Strobe tick OFF: hide strobe light
                strobeOff = true;

            } else if (strobeFxTickMs > 0 && (millis() - strobeFxTickMs <= strobeFxDurationMs || strobeOff)) {

                // Strobe tick ON: show strobe light for `strobeFxDurationMs` milliseconds (25)
                auto c = LightColor();
                c.setColor(0, 0, 1, 0);
                fx_solid(pixels, c, 0);
                strobeOff = false;

            } else {

                // Apply selected effect (FX.Style)

                auto currentColor = statusLed.getCurrentColor();
                // apply selected light style
                switch (currentStyleIndex) {
                    case LightStyles::RAINBOW:
                        fx_rainbow(pixels, currentColor, 1);
                        break;
                    case LightStyles::RAINBOW_2:
                        fx_rainbow(pixels, currentColor, 2);
                        break;
                    case LightStyles::WHITE_STRIPES:
                        fx_white_stripes(pixels, currentColor);
                        break;
                    case LightStyles::WHITE_STRIPES_2:
                        fx_white_stripes(pixels, currentColor, true);
                        break;
                    case LightStyles::KALEIDOSCOPE:
                        fx_kaleidoscope(pixels, currentColor);
                        break;
                    case LightStyles::SOLID:
                    default:
                        fx_solid(pixels, currentColor, strobeFxTickMs > 0 ? 0 : 200);
                        break;
                }

            }

            renderPixels();
            // show pixels
            refresh();

            lastRefreshTs = millis();
            if (strobeFxTickMs > 0 && strobeOff) {
                strobeFxTickMs = lastRefreshTs;
            }
        }

        // check for commands issued via physical button
        if (buttonCommand == BUTTON_COMMAND_STEP_DIM && millis() - lastStepCommandTs > BUTTON_COMMAND_STEP_INTERVAL) {
            lastStepCommandTs = millis();
            colorLight->dim(0);
        } else if (buttonCommand == BUTTON_COMMAND_STEP_BRIGHT && millis() - lastStepCommandTs > BUTTON_COMMAND_STEP_INTERVAL) {
            lastStepCommandTs = millis();
            colorLight->bright(0);
        }

    }
}
