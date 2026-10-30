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

#define COLOR_LIGHT_ADDRESS "C1"


using namespace Service::API::devices;
using namespace Service::API::WidgetApi;

class StatusLed: Task {
private:
    // Optional RGB Status LED
    Adafruit_NeoPixel* statusLED = nullptr;
    uint32_t effectColor = 0;

    // Status LED activity signal callback
    bool activityBlink = false;
    unsigned long activityBlinkTs = 0;
    LightColor currentColor;
    ColorLight* colorLight;

    void blinkLed(bool isLedOn) {
        if (statusLED == nullptr) return;
        if (isLedOn) {
            statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(100, 100, 0));
        } else {
            statusLED->setPixelColor(0, effectColor);
        }
        statusLED->show();
    }

public:
    StatusLed() {
        setLoopInterval(10);
    }

    void setup() {
#ifdef BOARD_HAS_RGB_LED
        // Get status LED config
        auto pin = Config::getSetting("stld-pin");
        int statusLedPin = pin.isEmpty() ? -1 : pin.toInt();
        if (statusLedPin >= 0) {
            int statusLedType = Config::getSetting("stld-typ", "82").toInt();
            int statusLedSpeed = Config::getSetting("stld-spd", "0").toInt();
            statusLED = new Adafruit_NeoPixel(1, statusLedPin, statusLedType + statusLedSpeed);
            statusLED->setPixelColor(0, 0, 0, 0);
            statusLED->begin();
            // Status LED Blink callback
            Config::statusLedCallback([this](bool isLedOn) {
                blinkLed(isLedOn);
            });
        }
#endif
        // Setup master LED control module
        colorLight = new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, COLOR_LIGHT_ADDRESS, "Color Light");
        colorLight->module->setProperty(Implements::Scheduling, "true");
        colorLight->module->setProperty(Implements::Scheduling_ModuleEvents, "true");

        colorLight->onSetColor([this](LightColor c) {
            currentColor.setColor(c.getHue(), c.getSaturation(), c.getValue(), 0);
            setEffectColor(c);
            if (statusLED != nullptr) statusLED->show();
        });

        auto hg = Service::HomeGenie::getInstance();
        hg->addAPIHandler(colorLight);
    }

    void loop() override {
        if (activityBlink && millis() - activityBlinkTs > 50) {
            activityBlink = false;
            if (statusLED != nullptr) {
                if (Config::isStatusLedOn) blinkLed(true);
                else statusLED->setPixelColor(0, effectColor);
                statusLED->show();
            }
        }
    }

    void signalActivity(uint8_t r, uint8_t g, uint8_t b) {
        if (statusLED != nullptr) {
            statusLED->setPixelColor(0, r, g, b);
            statusLED->show();
        }
        activityBlink = true;
        activityBlinkTs = millis();
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
        if (statusLED != nullptr && !Config::isStatusLedOn && !activityBlink) {
            statusLED->setPixelColor(0, effectColor);
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
