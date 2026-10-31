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

#ifndef HOMEGENIE_MINI_STATUSLED_H
#define HOMEGENIE_MINI_STATUSLED_H

#include <service/api/CommonApi.h>
#include <service/api/devices/ColorLight.h>

#include <Adafruit_NeoPixel.h>

#include "AuxiliaryLed.h"

#define COLOR_LIGHT_ADDRESS "C1"


using namespace Service::API::devices;
using namespace Service::API::WidgetApi;

class StatusLed: Task {
private:
    // Optional RGB Status LED
    Adafruit_NeoPixel* statusLED = nullptr;
    AuxiliaryLed* auxiliaryLed = nullptr;
    uint32_t effectColor = 0;
    bool helloWorldActive = true;
    bool statusSignalOn = false;

    // Status LED activity signal callback
    bool activityBlink = false;
    unsigned long activityBlinkTs = 0;
    LightColor currentColor;
    ColorLight* colorLight;

    void writeColor(uint32_t color) {
        if (statusLED != nullptr) statusLED->setPixelColor(0, color);
        if (auxiliaryLed != nullptr) auxiliaryLed->setStatusColor(color);
    }

    void blinkLed(bool isLedOn) {
        // Keep the historical 20-second startup window for connected color-light.
        // Loss and explicit setup must still show, including the held-button preview.
        statusSignalOn = isLedOn && (helloWorldActive || !WiFi.isConnected()
            || Config::statusLedConfigurationActive);
        writeColor(statusSignalOn ? Adafruit_NeoPixel::Color(100, 100, 0) : effectColor);
        show();
    }

public:
    StatusLed() {
        setLoopInterval(10);
    }

    void setup() {
        int statusLedPin = -1;
#ifdef BOARD_HAS_RGB_LED
        // Get status LED config
        auto pin = Config::getSetting("stld-pin");
        statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
        if (statusLedPin >= 0) {
            int statusLedType = Config::getSetting("stld-typ", "82").toInt();
            int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
            statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
            statusLED->setPixelColor(0, 0, 0, 0);
            statusLED->begin();
        }
#endif
        // Setup master LED control module
        colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Color Light");
        colorLight->module->setProperty(Implements::Scheduling, "true");
        colorLight->module->setProperty(Implements::Scheduling_ModuleEvents, "true");

        colorLight->onSetColor([this](LightColor c) {
            currentColor.setColor(c.getHue(), c.getSaturation(), c.getValue(), 0);
            setEffectColor(c);
            show();
        });

        auto hg = Service::HomeGenie::getInstance();
        hg->addAPIHandler(colorLight);

        // Allocate the optional LED/module only when explicitly configured.
        if (Config::getSetting("auld-pin", "-1").toInt() >= 0) {
            auxiliaryLed = new AuxiliaryLed();
            if (!auxiliaryLed->setup(statusLedPin)) {
                delete auxiliaryLed;
                auxiliaryLed = nullptr;
            }
        }
        // One callback drives both indicators, including aux-only boards.
        if (statusLED != nullptr || auxiliaryLed != nullptr) {
            Config::statusLedCallback([this](bool isLedOn) { blinkLed(isLedOn); });
            blinkLed(Config::isStatusLedOn);
        }
    }

    void loop() override {
        if (helloWorldActive && millis() > 20000 && Config::isDeviceConfigured()) {
            helloWorldActive = false;
            if (!activityBlink) blinkLed(Config::isStatusLedOn);
        }
        if (activityBlink && millis() - activityBlinkTs > 50) {
            activityBlink = false;
            blinkLed(Config::isStatusLedOn);
        }
    }

    void signalActivity(uint8_t r, uint8_t g, uint8_t b) {
        writeColor(Adafruit_NeoPixel::Color(r, g, b));
        show();
        activityBlink = true;
        activityBlinkTs = millis();
    }

    void show() {
        if (statusLED != nullptr) statusLED->show();
        if (auxiliaryLed != nullptr) auxiliaryLed->show();
    }

    bool isEnabled() {
        return statusLED != nullptr;
    }

    void setLED(Adafruit_NeoPixel* led) {
        statusLED = led;
    }
    Adafruit_NeoPixel* getLed() {
        return statusLED;
    }
    ColorLight* getColorLight() {
        return colorLight;
    }

    void setEffectColor(const LightColor& c) {
        effectColor = c.getColor();
        if (!statusSignalOn && !activityBlink) {
            writeColor(effectColor);
        }
    }

    void setCurrentColor(LightColor c) {
        currentColor = c;
    }
    LightColor& getCurrentColor() {
        return currentColor;
    }

};

#endif //HOMEGENIE_MINI_STATUSLED_H
