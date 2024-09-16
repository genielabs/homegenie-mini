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

    // Get a reference to HomeGenie Mini running instance
    homeGenie = HomeGenie::getInstance();

    // Get the default system module
    auto miniModule = homeGenie->getDefaultModule();
    miniModule->name = "Smart light";

    // Add to the default module some configuration properties.
    // Properties starting with `Widget.OptionField.` are special
    // properties that are used in the HomeGenie Panel app to
    // display custom buttons, sliders, selectors and other UI controls
    // to configure parameters of this device implementation

    // Number select control to set the number of LEDs of the strip
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.LED.count",
                                "number:LED.count:1:1920:1:led_count"));
    // Number select control to set maximum percentage of power that can be drawn
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.LED.power",
                                "number:LED.power:5:100:25:led_power"));
    // Dropdown list control to select the light animation effect
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Rainbow",
                                "select:FX.Style:light_style:solid|rainbow|rainbow_2|white_stripes|white_stripes_2|kaleidoscope"));
    // Dropdown list control to enable the strobe effect
    miniModule->properties.add(
            new ModuleParameter("Widget.OptionField.FX.Strobe",
                                "select:FX.Strobe:strobe_effect:off|slow|medium|fast"));

    // The following are the actual properties where
    // UI controls implemented by `Widget.Options`
    // read/write values from/to.
    mpLedCount = new ModuleParameter("LED.count", "0");
    miniModule->properties.add(mpLedCount);
    mpMaxPower = new ModuleParameter("LED.power", "25");
    miniModule->properties.add(mpMaxPower);
    mpFxStyle = new ModuleParameter("FX.Style", lightStyleNames[0]);
    miniModule->properties.add(mpFxStyle);
    mpFxStrobe = new ModuleParameter("FX.Strobe", "off");
    miniModule->properties.add(mpFxStrobe);

    // Get status LED config
    int statusLedPin = Config::getSetting("stld-pin", "-1").toInt();
    if (statusLedPin >= 0) {
        int statusLedType = Config::getSetting("stld-typ", "82").toInt();
        int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
        statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
        statusLED->setPixelColor(0, 0, 0, 0);
        statusLED->begin();
    }

    isConfigured = Config::isDeviceConfigured();
    if (!isConfigured) {

        // Custom status led (builtin NeoPixel RGB LED)
        if (statusLED != nullptr) {
            Config::statusLedCallback(&statusLedCallback);
        }

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

        mpLedCount->value = String(ledsCount);
        mpMaxPower->value = String(maxPower);

        // Setup main LEDs control module
        mainModule = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, "C1", "Main");
        mainModule->module->properties.add(
                new ModuleParameter("Widget.Preference.AudioLight", "true"));
        mainModule->onSetColor([](LightColor color) {
            currentColor = color;
            fx_reset(pixels, color);
        });
        homeGenie->addAPIHandler(mainModule);

        // Setup control buttons
        setupControlButtons(miniModule);

        // Initialize FX buffer
        fx_init(ledsCount, currentColor);

        // TODO: implement color/status recall on start
        // Set default color
        float h = presetColors[0][0];
        float s = presetColors[0][1];
        mainModule->setColor(h, s, 0, 0);

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
            mainModule->dim(0);
        } else if (buttonCommand == BUTTON_COMMAND_STEP_BRIGHT && millis() - lastStepCommandTs > BUTTON_COMMAND_STEP_INTERVAL) {
            lastStepCommandTs = millis();
            mainModule->bright(0);
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
