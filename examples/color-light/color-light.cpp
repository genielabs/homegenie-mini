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

    // Add to the default module some configuration properties.
    // Properties starting with `Widget.OptionField.` are special
    // properties that are used in the HomeGenie Panel app to
    // display custom buttons, sliders, selectors and other UI controls
    // to configure parameters of this device implementation

    // Number select control to set the number of LEDs of the strip
    miniModule->setProperty("Widget.OptionField.LED.count",
                            "number:LED.count:1:1920:1:led_count");
    // Number select control to set maximum percentage of power that can be drawn
    miniModule->setProperty("Widget.OptionField.LED.power",
                            "number:LED.power:5:100:25:led_power");

    // The following are the actual properties where
    // UI controls implemented by `Widget.Options`
    // read/write values from/to.
    mpLedCount = new ModuleParameter("LED.count", "0");
    miniModule->properties.add(mpLedCount);
    mpMaxPower = new ModuleParameter("LED.power", "25");
    miniModule->properties.add(mpMaxPower);

    // Get status LED config
    auto pin = Config::getSetting("stld-pin");
    int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ", "82").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
        statusLED->begin();
    }

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

        // Get LED strip config
        ledsCount = abs(Config::getSetting("leds-cnt").toInt());
        ledsPin = (int16_t)Config::getSetting("leds-pin").toInt();
        pixelsType = (int16_t)Config::getSetting("leds-typ").toInt();
        pixelsSpeed = (int16_t)Config::getSetting("leds-spd").toInt();
        maxPower = Config::getSetting("leds-pwr").toInt();
        if (maxPower <= 0) maxPower = DEFAULT_MAX_POWER;
        createPixels();
        // default values
        mpLedCount->value = String(ledsCount);
        mpMaxPower->value = String(maxPower);

        // Setup main LEDs control module
        colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Color Light");
        colorLight->module->setProperty("Widget.Implements.Scheduling", "1");
        colorLight->module->setProperty("Widget.Implements.Scheduling.ModuleEvents", "1");
        colorLight->module->setProperty("Widget.Preference.AudioLight", "true");
        colorLight->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(pixels, color);
        });
        homeGenie->addAPIHandler(colorLight);

        auto module = colorLight->module;
        module->name = "Smart Light";

        // Add to the ColorLight module some configuration properties:
        // - dropdown list control to select the light animation effect
        module->setProperty("Widget.OptionField.FX.Rainbow",
                            "select:FX.Style:light_style:solid|rainbow|rainbow_2|white_stripes|white_stripes_2|kaleidoscope");
        // - dropdown list control to enable the strobe effect
        module->setProperty("Widget.OptionField.FX.Strobe",
                            "select:FX.Strobe:strobe_effect:off|slow|medium|fast");

        mpFxStyle = new ModuleParameter("FX.Style", lightStyleNames[0]);
        module->properties.add(mpFxStyle);
        mpFxStrobe = new ModuleParameter("FX.Strobe", "off");
        module->properties.add(mpFxStrobe);

        // Setup control buttons
        setupControlButtons(module);

#ifndef DISABLE_AUTOMATION
        // Setup example schedules/scenes
        setupDefaultSchedules();
#endif

        // Initialize FX buffer
        fx_init(ledsCount, currentColor);

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

    // Custom status led (builtin NeoPixel RGB LED)
    statusLedLoop();

    if (isConfigured) {

        if (millis() - lastRefreshTs > refreshMs)
        {
            // Update current rendering style if changed
            uint8_t selectedStyleIndex = getLightStyleIndex(mpFxStyle->value);
            if (currentStyleIndex != selectedStyleIndex) {
                currentStyleIndex = selectedStyleIndex;
            }

            // enable / disable strobe light
            if (strobeFxTickMs > 0 && mpFxStrobe->value == "off") {
                strobeFxTickMs = 0;
            } else if (strobeFxTickMs == 0 && mpFxStrobe->value != "off") {
                strobeFxTickMs = millis();
            }

            if (!strobeOff && strobeFxTickMs > 0 && millis() - strobeFxTickMs > ((strobeFxDurationMs + strobeFxIntervalMs) * (mpFxStrobe->value == "slow" ? 3 : mpFxStrobe->value == "medium" ? 2 : 1))) {

                // strobe off
                strobeOff = true;

            } else if (strobeFxTickMs > 0 && (millis() - strobeFxTickMs <= strobeFxDurationMs || strobeOff)) {

                // show strobe light for `strobeFxDurationMs` milliseconds (25)
                auto c = LightColor();
                c.setColor(0, 0, 1, 0);
                fx_solid(pixels, c, 0);
                strobeOff = false;

            } else {

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
            if (strobeOff) {
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

        // check if number of pixels was changed
        if (ledsCount != mpLedCount->value.toInt()) {

            disposePixels();

            ledsCount = mpLedCount->value.toInt();
            Config::saveSetting("leds-cnt", mpLedCount->value);

            // re-create pixels with the new length
            createPixels();

        }
        // check if max power value was changed
        if (maxPower != mpMaxPower->value.toInt() && mpMaxPower->value.toInt() > 0) {
            maxPower = mpMaxPower->value.toInt();
            Config::saveSetting("leds-pwr", mpMaxPower->value);
        }

    }
}
