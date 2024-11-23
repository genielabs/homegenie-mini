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

#include <HomeGenie.h>
#include <service/api/devices/Button.h>

#include <Adafruit_NeoPixel.h>

#include "configuration.h"
#include "color-fx.h"
#include "status-led.h"

using namespace Service;
using namespace Service::API::devices;

HomeGenie* homeGenie;
bool isConfigured = false;

// LED strip
uint16_t ledsCount = 0; int16_t ledsPin = -1;
int16_t pixelsType, pixelsSpeed;
Adafruit_NeoPixel* pixels = nullptr;
uint8_t maxPower = DEFAULT_MAX_POWER;

// Preset colors (normalized HSV)
const int presetColorsCount = 7;
float presetColors[presetColorsCount][3] = {
    { 202 / 360.0f, 1, 0.61 }, // Dodger Blue
    {            0, 0,    1 }, // White
    {  23 / 360.0f, 1, 0.65 }, // Coral
    { 123 / 360.0f, 1, 0.42 }, // Fun Green
    {   1 / 360.0f, 1, 0.54 }, // Scarlet
    {  48 / 360.0f, 1, 0.67 }, // Mustard
    { 260 / 360.0f, 1, 0.66 }  // Heliotrope
};
uint8_t presetColorIndex = 0;

// Light animation styles
enum LightStyles {
    SOLID = 0,
    RAINBOW,
    RAINBOW_2,
    WHITE_STRIPES,
    WHITE_STRIPES_2,
    KALEIDOSCOPE,
    STYLES_COUNT // this must be always the last element
};
const char* const lightStyleNames[] = {
        "solid",
        "rainbow",
        "rainbow_2",
        "white_stripes",
        "white_stripes_2",
        "kaleidoscope"
};
uint8_t getLightStyleIndex(String& styleName) {
    for (int j = 0; j < LightStyles::STYLES_COUNT; j++) {
        if  (styleName == lightStyleNames[j]) {
            return j;
        }
    }
    return 0;
}

ColorLight* colorLight;
LightColor currentColor;

ModuleParameter* mpLedCount;
ModuleParameter* mpMaxPower;
ModuleParameter* mpFxStyle;
ModuleParameter* mpFxStrobe;

// Buttons state variables
unsigned long lastStepCommandTs = -1;
uint8_t buttonCommand = -1;

// Light animation effects state variables
unsigned int refreshMs = 10;
unsigned long lastRefreshTs = 0;
unsigned long strobeFxTickMs = 0;
unsigned int strobeFxDurationMs = 20;
unsigned int strobeFxIntervalMs = 80;  // limit strobe to 10Hz  (20 + 80 -> 100ms interval)
bool strobeOff = true;
uint8_t currentStyleIndex = 0;

void refresh() {
    if (statusLED != nullptr) {
        statusLED->show();
    }
    if (pixels != nullptr) {
        pixels->show();
    }
}

void renderPixels() {
    if (pixels != nullptr) {
        float pwr = ((float)maxPower / 100.0f);
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i,
                                  static_cast<int>(round(animatedColors[i]->getRed() * pwr)),
                                  static_cast<int>(round(animatedColors[i]->getGreen() * pwr)),
                                  static_cast<int>(round(animatedColors[i]->getBlue() * pwr)));
        }
    }
    if (statusLED != nullptr) {
        statusLED->setPixelColor(0,
                                 static_cast<int>(round(animatedColors[0]->getRed())),
                                 static_cast<int>(round(animatedColors[0]->getGreen())),
                                 static_cast<int>(round(animatedColors[0]->getBlue())));
    }
}

void createPixels()
{
    if (ledsPin >= 0) {
        pixels = new Adafruit_NeoPixel(ledsCount, ledsPin, pixelsType + pixelsSpeed);
        // turn off
        //pixels->clear();
        //*
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }
        //*/
        pixels->begin();
    }
}
void disposePixels() {

    // turn off all pixels
    if (pixels != nullptr) {
        //pixels->clear();
        //*
        for (int i = 0; i < ledsCount; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }
        refresh();
        //*/
        delete pixels;
        delay(1000);
    }

}

// Adds 2 "GpioButton" modules for handling manual control (on/off, level, switch fx, ...)
void setupControlButtons(Module* miniModule) {
    static bool button1Pressed = false;

    auto btn1_pin = Config::getSetting("btn1-pin", "-1").toInt();
    if (btn1_pin >= 0) {
        auto button1 = new Button(IO::IOEventDomains::HomeAutomation_HomeGenie, "B1", "Button 1", btn1_pin);
        button1->onSetStatus([](Service::API::devices::ButtonStatus status) {
            if (status == BUTTON_STATUS_PRESSED) {
                button1Pressed = true;
            } else {
                button1Pressed = false;
                buttonCommand = BUTTON_COMMAND_NONE;
            }
        });
        button1->onGesture([](Service::API::devices::ButtonGesture gesture) {
            if (buttonCommand != BUTTON_COMMAND_NONE) return;
            switch (gesture) {
                case ButtonGesture::BUTTON_GESTURE_CLICK: {
                        colorLight->toggle();
                    }
                    break;
                case ButtonGesture::BUTTON_GESTURE_LONG_PRESS: {
                        if (colorLight->isOn()) {
                            buttonCommand = BUTTON_COMMAND_STEP_DIM;
                        }
                    }
                    break;
            }
        });
    }
    auto btn2_pin = Config::getSetting("btn2-pin", "-1").toInt();
    if (btn2_pin >= 0) {
        auto button2 = new Button(IO::IOEventDomains::HomeAutomation_HomeGenie, "B1", "Button 1", btn2_pin);
        button2->onSetStatus([](Service::API::devices::ButtonStatus status) {
            if (status == BUTTON_STATUS_PRESSED) {
                if (button1Pressed && (buttonCommand == BUTTON_COMMAND_NONE || buttonCommand == BUTTON_COMMAND_PRESET_COLORS)) {
                    // Rotate preset colors
                    buttonCommand = BUTTON_COMMAND_PRESET_COLORS;
                    float h = presetColors[presetColorIndex][0];
                    float s = presetColors[presetColorIndex][1];
                    float v = presetColors[presetColorIndex][2];
                    colorLight->setColor(h, s, v, 0);
                    presetColorIndex = (presetColorIndex + 1) % presetColorsCount;
                }
            } else if (!button1Pressed) {
                buttonCommand = BUTTON_COMMAND_NONE;
            }
        });
        button2->onGesture([miniModule](Service::API::devices::ButtonGesture gesture) {
            if (buttonCommand != BUTTON_COMMAND_NONE || button1Pressed || !colorLight->isOn()) return;
            switch (gesture) {
                case ButtonGesture::BUTTON_GESTURE_CLICK: {
                        currentStyleIndex = (currentStyleIndex + 1) % LightStyles::STYLES_COUNT;
                        auto msg = QueuedMessage(miniModule, "FX.Style", lightStyleNames[currentStyleIndex], nullptr,
                                                 IOEventDataType::Undefined);
                        miniModule->setProperty("FX.Style", lightStyleNames[currentStyleIndex], nullptr, IOEventDataType::Undefined);
                        HomeGenie::getInstance()->getEventRouter().signalEvent(msg);
                    }
                    break;
                case ButtonGesture::BUTTON_GESTURE_LONG_PRESS: {
                        buttonCommand = BUTTON_COMMAND_STEP_BRIGHT;
                    }
                    break;
            }
        });
    }

}

#ifndef DISABLE_AUTOMATION
void setupDefaultSchedules() {

    // Create default Dawn scene
    if (Scheduler::get("Dawn") == nullptr) {
        auto s = new Schedule("Dawn", "Dawn scene colors, 24 minutes before sunrise.", "", "@SolarTimes.Sunrise - 24", "$$.boundModules.on();");
        s->onModuleEvent = false;
        //s->eventModules.add(miniModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{"color":"#FF9100|30.0"},"enabled":true,"id":"command_set_color"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[],"from":"","itemType":3,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
        Scheduler::addSchedule(s);
    }

    // Create default Dusk scene
    if (Scheduler::get("Dusk") == nullptr) {
        auto s = new Schedule("Dusk", "Dusk scene colors, 42 minutes after sunset.", "", "@SolarTimes.Sunset + 42", "$$.boundModules.on();");
        s->onModuleEvent = false;
        //s->eventModules.add(miniModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{"color":"#0099FF|30.0"},"enabled":true,"id":"command_set_color"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[],"from":"","itemType":3,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
        Scheduler::addSchedule(s);
    }

    // Create default Light.OFF scene
    if (Scheduler::get("Lights.Off") == nullptr) {
        auto s = new Schedule("Lights.Off", "Ensure lights are turned off every day at 2AM.", "", "0 2 * * *", "$$.boundModules.off();");
        s->onModuleEvent = false;
        //s->eventModules.add(miniModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{},"enabled":true,"id":"command_turn_off"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[],"from":"","itemType":1,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[{"end":"02:00","start":"02:00"}],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String("Light"));
        s->boundDevices.add(new String("Dimmer"));
        s->boundDevices.add(new String("Color"));
        Scheduler::addSchedule(s);
    }

    if (true || Scheduler::getScheduleList().size() == 0) {

        // TODO: if added at least one schedule then  ->  Scheduler::save();

    }

}
#endif // DISABLE_AUTOMATION