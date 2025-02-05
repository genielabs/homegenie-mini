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
    bool _statusLedWifiConnected = false;

    // Blinks only first 20 seconds right after boot
    unsigned long helloWorldDuration = 20000;
    bool helloWorldActive = true;

    // Status LED activity signal callback
    bool signalRfReceive = false;
    unsigned long signalRfReceiveTs = 0;
    LightColor currentColor;
    ColorLight* colorLight;

    void blinkLed(bool isLedOn) {
        if (statusLED == nullptr) return;
        if (isLedOn) {
            statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(100, 100, 0));
        } else {
            statusLED->setPixelColor(0, Adafruit_NeoPixel::Color(0, 0, 0));
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
        colorLight->module->setProperty(Preference::AudioLight, "true");

        colorLight->onSetColor([this](LightColor c) {
            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, c.getRed(), c.getGreen(), c.getBlue());
                statusLED->show();
                currentColor.setColor(c.getHue(), c.getSaturation(), c.getValue(), 0);
            }
        });

        auto hg = Service::HomeGenie::getInstance();
        hg->addAPIHandler(colorLight);
    }

    void loop() override {
        if (statusLED != nullptr) {
            if (helloWorldActive && millis() > helloWorldDuration && Config::isDeviceConfigured()) {
                // stops blinking if "helloWorldDuration" is elapsed
                helloWorldActive = false;
                Config::statusLedCallback(nullptr);
            } else if (WiFi.isConnected() != _statusLedWifiConnected) {
                _statusLedWifiConnected = WiFi.isConnected();
                if (_statusLedWifiConnected) {
                    // turn off status LED blinking if connected
                    if (!helloWorldActive) {
                        Config::statusLedCallback(nullptr);
                    }
                } else {
                    // blinks status LED rapidly if not connected
                    Config::statusLedCallback([this](bool isLedOn) {
                        blinkLed(isLedOn);
                    });
                }
            }
        }

        if (signalRfReceive && millis() - signalRfReceiveTs > 50) {
            signalRfReceive = false;
            if (statusLED != nullptr) {
                statusLED->setPixelColor(0, currentColor.getRed(), currentColor.getGreen(), currentColor.getBlue());
                statusLED->show();
            }
        }
    }

    void signalActivity(uint8_t r, uint8_t g, uint8_t b) {
        if (statusLED != nullptr) {
            statusLED->setPixelColor(0, r, g, b);
            statusLED->show();
        }
        signalRfReceive = true;
        signalRfReceiveTs = millis();
    }

    bool isEnabled() {
        return statusLED != nullptr;
    }
    Adafruit_NeoPixel* getLed() {
        return statusLED;
    }
    ColorLight* getColorLight() {
        return colorLight;
    }
    LightColor& getCurrentColor() {
        return currentColor;
    }
    void setCurrentColor(LightColor c) {
        currentColor = c;
    }

};

#endif //HOMEGENIE_MINI_STATUSLED_H
