/*
 * HomeGenie-Mini (c) 2018-2026 G-Labs
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

    // Read stored config (common)
    ledsPin = (int16_t)Config::getSetting(Configuration::LedsPin).toInt();

    // the following settings can also be configured from the app
    ledsConfig.count = abs(Config::getSetting(Configuration::LedsCount).toInt());
    if (ledsConfig.count < 1) ledsConfig.count = 1;
    ledsConfig.rows = abs(Config::getSetting(Configuration::LedsRows).toInt());
    if (ledsConfig.rows < 1) ledsConfig.rows = 1;
    ledsConfig.cols = ledsConfig.count / ledsConfig.rows;
    ledsConfig.isVertical = Config::getSetting(Configuration::LedsIsVert, "false").equals("true");
    ledsConfig.isSerpentine = Config::getSetting(Configuration::LedsIsSerp, "true").equals("true");
    ledsConfig.startAtBottom = Config::getSetting(Configuration::LedsStBottom, "false").equals("true");
    ledsConfig.startAtRight = Config::getSetting(Configuration::LedsStRight, "false").equals("true");

    maxPower = Config::getSetting(Configuration::LedsPower).toInt();
    if (maxPower <= 0) maxPower = DEFAULT_MAX_POWER;

    // NeoPixels WS2810B RGB LED config
    pixelsType = (int16_t)Config::getSetting(Configuration::LedsType).toInt();
    pixelsSpeed = (int16_t)Config::getSetting(Configuration::LedsSpeed).toInt();

    // Add UI options to configure this device using the app
    setupControllerOptions(miniModule);

    // Setup builtin RGB status led
    statusLed.setup();
    colorLight = statusLed.getColorLight();

    isConfigured = Config::isDeviceConfigured();
    // Provisioning is an explicit session until reboot. A network event alone
    // must not end it (e.g. an SDK reconnect using the retained credentials).
    if (isConfigured) {

        createPixels();

        // colorLight is the main control module
        colorLight->onSetColor([](LightColor color) {
            statusLed.setCurrentColor(color);
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

        scriptHandler = new LiteScriptHandler(module);
        homeGenie->addAPIHandler(scriptHandler);
#endif

        // Initialize FX buffer
        fx_init(ledsConfig, statusLed.getCurrentColor());

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

        // Track the effect style switch
        static int lastStyleIndex = -1;
        if (currentStyleIndex != lastStyleIndex) {
            lastStyleIndex = currentStyleIndex;
#ifndef DISABLE_AUTOMATION
            scriptHandler->startTime = millis(); // Force fx_script to reset on next activation
#endif
            styleChanged = true;
            levelChanged = true;                 // Force redraw/refresh
        }

        if (millis() - lastRefreshTs > refreshMs)
        {

            if (!strobeOff && strobeFxTickMs > 0 && millis() - strobeFxTickMs > ((strobeFxDurationMs + strobeFxIntervalMs) * (mpFxStrobe->value == "slow" ? 3 : mpFxStrobe->value == "medium" ? 2 : 1))) {

                // Strobe tick OFF: hide strobe light
                strobeOff = true;

            } else if (strobeFxTickMs > 0 && (millis() - strobeFxTickMs <= strobeFxDurationMs || strobeOff)) {

                // Strobe tick ON: show strobe light for `strobeFxDurationMs` milliseconds (25)
                auto c = LightColor();
                c.setColor(0, 0, 1, 0);
                fx_solid(ledsConfig, c, 0);
                strobeOff = false;

            } else {

                // Apply selected effect (FX.Style)
                uint16_t rows = ledsConfig.rows > 0 ? ledsConfig.rows : 1;
                uint16_t cols = ledsConfig.count / rows;
                if (cols == 0) cols = 1;

                auto currentColor = statusLed.getCurrentColor();
                switch (currentStyleIndex) {
                    case RAINBOW:
                        fx_rainbow(ledsConfig, currentColor, 1);
                        break;
                    case RAINBOW_2:
                        fx_rainbow(ledsConfig, currentColor, 2);
                        break;
                    case WHITE_STRIPES:
                        fx_white_stripes(ledsConfig, currentColor);
                        break;
                    case WHITE_STRIPES_2:
                        fx_white_stripes(ledsConfig, currentColor, true);
                        break;
                    case KALEIDOSCOPE:
                        fx_kaleidoscope(ledsConfig, currentColor);
                        break;
                    case DRIFT:
                        fx_drift(ledsConfig, currentColor);
                        break;
                    case FIRE:
                        fx_fire(ledsConfig, currentColor);
                        break;
                    case METEOR:
                        if (styleChanged) fx_clear(ledsConfig);
                        fx_meteor(ledsConfig, currentColor);
                        break;
                    case TWINKLE:
                        fx_twinkle(ledsConfig, currentColor);
                        break;
                    case MATRIX_RAIN:
                        fx_matrix_rain(ledsConfig, currentColor);
                        break;
                    case TUNNEL_3D:
                        fx_3d_tunnel(ledsConfig, currentColor);
                        break;
                    case DISTORTION_WAVES:
                        fx_distortion_waves(ledsConfig, currentColor);
                        break;
                    case PARTICLE_VORTEX:
                        fx_vortex(ledsConfig, currentColor);
                        break;
                    case WATERFALL:
                        fx_waterfall(ledsConfig, currentColor);
                        break;
                    case PARTICLE_GEQ:
                        fx_particle_geq(ledsConfig, currentColor);
                        break;
                    case PARTICLE_CENTER_GEQ:
                        fx_particle_center_geq(ledsConfig, currentColor);
                        break;
                    case BLACK_HOLE:
                        fx_black_hole(ledsConfig, currentColor);
                        break;
                    case POLAR_LIGHT:
                        fx_polar_lights(ledsConfig, currentColor);
                        break;
                    case WAVERLY:
                        fx_waverly(ledsConfig, currentColor);
                        break;
#ifndef DISABLE_AUTOMATION
                    case SCRIPT: {
                        if (scriptHandler->compiledScript.size() == 0) {
                            String fallbackScript = F(
                                "#1\n"
                                "00.000 ALL DEFAULT(1.00,1500)\n"
                                "01.500 ALL DEFAULT(0.00,1500)\n"
                                "03.000 ALL Off\n"
                                "#LOOP:1"
                            );
                            compileScript(fallbackScript, scriptHandler->compiledScript, ledsConfig);
                        }
                        auto isPlaying = fx_script(ledsConfig, currentColor, scriptHandler->compiledScript, scriptHandler->startTime);
                        if (!isPlaying || levelChanged) {
                            // Restart / Redraw
                            scriptHandler->startTime = millis();
                        }
                        break;
                    }
#endif
                    case SOLID:
                    default:
                        fx_solid(ledsConfig, currentColor, strobeFxTickMs > 0 ? 0 : 200);
                        break;
                }
                levelChanged = false;
                styleChanged = false;
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
