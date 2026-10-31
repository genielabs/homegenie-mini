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
#pragma once

#include <HomeGenie.h>
#include <service/api/devices/Button.h>

#include <Adafruit_NeoPixel.h>
#include <atomic>

#include "configuration.h"

#include "fx/fx.h"
#include "fx/fx_3d_tunnel.h"
#include "fx/fx_black_hole.h"
#include "fx/fx_distortion_waves.h"
#include "fx/fx_drift.h"
#include "fx/fx_fire.h"
//#include "fx/fx_fireworks.h"
#include "fx/fx_kaleidoscope.h"
#include "fx/fx_matrix_rain.h"
#include "fx/fx_meteor.h"
#include "fx/fx_particle_geq.h"
#include "fx/fx_particle_center_geq.h"
#include "fx/fx_polar_lights.h"
#include "fx/fx_rainbow.h"
#ifndef DISABLE_AUTOMATION
#include "fx/fx_script.h"
#endif
#include "fx/fx_twinkle.h"
#include "fx/fx_vortex.h"
#include "fx/fx_waverly.h"
#include "fx/fx_waterfall.h"
#include "fx/fx_white_stripes.h"

#include "StatusLed.h"
StatusLed statusLed;

#ifndef DISABLE_AUTOMATION
#include "api/LiteScriptHandler.h"
#endif

namespace ColorLightApi {
    namespace Options {
        namespace Controller {
            const char LED_count[] = "LED.count";
            const char LED_rows[]  = "LED.rows";
            const char LED_vert[]  = "LED.vert";
            const char LED_serp[]  = "LED.serp";
            const char LED_strg[]  = "LED.strg";
            const char LED_stbt[]  = "LED.stbt";
            const char LED_power[] = "LED.power";
        }
        namespace Light {
            const char FX_Style[] = "FX.Style";
            const char FX_Strobe[] = "FX.Strobe";
        }
    }

    namespace Configuration {
        static const char LedsPin[]      = "leds-pin";
        static const char LedsType[]     = "leds-typ";
        static const char LedsSpeed[]    = "leds-spd";
        static const char LedsCount[]    = "leds-cnt"; // count
        static const char LedsRows[]     = "leds-rws"; // rows
        static const char LedsIsVert[]   = "leds-vrt"; // isVertical
        static const char LedsIsSerp[]   = "leds-srp"; // isSerpentine
        static const char LedsStBottom[] = "leds-stb"; // startAtBottom
        static const char LedsStRight[]  = "leds-str"; // startAtRight
        static const char LedsPower[]    = "leds-pwr";
        static const char LedsModules[]  = "leds-mod";
    }

}

using namespace Service;
using namespace Service::API::devices;
using namespace Service::ModuleApi;

using namespace ColorLightApi;

HomeGenie* homeGenie;
bool isConfigured = false;

#ifndef DISABLE_AUTOMATION
LiteScriptHandler* scriptHandler = nullptr;
#endif

// Master/Status LED
ColorLight* colorLight = nullptr;

// LED strip
MatrixConfig ledsConfig = { 0, 0, 1, false, true, false, false }; // 16x16
int16_t ledsPin = -1;
uint8_t maxPower = DEFAULT_MAX_POWER;
// WS2810B RGB LED
int16_t pixelsType;
int16_t pixelsSpeed;
Adafruit_NeoPixel* pixels = nullptr;
// API callbacks publish only the requested size. Allocation and publication of
// the new render state happen together in the Arduino loop, between frames.
std::atomic<uint16_t> requestedLedCount{0};

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
    DISTORTION_WAVES = 0,
    SOLID,
    RAINBOW,
    RAINBOW_2,
    WHITE_STRIPES,
    WHITE_STRIPES_2,
    KALEIDOSCOPE,
    DRIFT,
    FIRE,
    METEOR,
    TWINKLE,
    MATRIX_RAIN,
    TUNNEL_3D,
    PARTICLE_VORTEX,
    WATERFALL,
    PARTICLE_GEQ,
    PARTICLE_CENTER_GEQ,
    BLACK_HOLE,
    POLAR_LIGHT,
    WAVERLY,
#ifndef DISABLE_AUTOMATION
    SCRIPT,
#endif
    STYLES_COUNT // this must be always the last element
};

const char* const lightStyleNames[] = {
    "distortion_waves",
    "solid",
    "rainbow",
    "rainbow_2",
    "white_stripes",
    "white_stripes_2",
    "kaleidoscope",
    "drift",
    "fire",
    "meteor",
    "twinkle",
    "matrix_rain",
    "tunnel_3d",
    "particle_vortex",
    "waterfall",
    "particle_geq",
    "particle_center_geq",
    "black_hole",
    "polar_light",
    "waverly",
#ifndef DISABLE_AUTOMATION
    "script"
#endif
};

inline uint8_t getLightStyleIndex(String& styleName) {
    for (int j = 0; j < STYLES_COUNT; j++) {
        if  (styleName == lightStyleNames[j]) {
            return j;
        }
    }
    return 0;
}

ModuleParameter* mpFxStrobe;

// Buttons state variables
unsigned long lastStepCommandTs = 0;
uint8_t buttonCommand = BUTTON_COMMAND_NONE;

// Light animation effects state variables
unsigned int refreshMs = 10;
unsigned long lastRefreshTs = 0;
unsigned long strobeFxTickMs = 0;
unsigned int strobeFxDurationMs = 20;
unsigned int strobeFxIntervalMs = 80;  // limit strobe to 10Hz  (20 + 80 -> 100ms interval)
bool strobeOff = true;
// Waves is first and is the default; named FX.Style identifiers stay unchanged.
uint8_t currentStyleIndex = DISTORTION_WAVES;

bool levelChanged = false;
bool styleChanged = false;

LinkedList<ColorLight*> colorLightList;

inline void refresh() {
    statusLed.show();
    if (pixels != nullptr) {
        pixels->show();
    }
}

inline void setupPixelModules()
{
    if (ledsPin >= 0) {
        if (Config::getSetting(Configuration::LedsModules, "false").equals("true")) {
            for (int i = colorLightList.size(); i < ledsConfig.count; i++) {
                auto address = String("L") + String(i + 1);
                auto name = String("LED ") + String(i + 1);
                colorLightList.add(new ColorLight(IO::IOEventDomains::HomeAutomation_HomeGenie, address.c_str(), name.c_str()));
                colorLightList[i]->onSetColor([i](LightColor color) {
                    // Handlers keep their identity across size changes. LEDs
                    // outside the current output must never touch its buffer.
                    if (pixels == nullptr || i >= ledsConfig.count) return;
                    float pwr = ((float)maxPower / 100.0f);
                    auto r = color.getRed() * pwr;
                    auto g = color.getGreen() * pwr;
                    auto b = color.getBlue() * pwr;
                    pixels->setPixelColor(i, r, g, b);
                    refresh();
                });
                homeGenie->addAPIHandler(colorLightList[i]);
            }
        }
    }
}

inline void renderPixels() {
    float pwr = ((float)maxPower / 100.0f);
    const LightColor* representative = nullptr;
    for (int i = 0; i < ledsConfig.count; i++) {
        auto color = animatedColors[i];
        if (color == nullptr) continue;
        uint32_t renderedColor = color->getColor(pwr);
        // Sample actual effect output, not just pixel zero (often black in
        // sparse effects). Reuse the render traversal without allocating.
        if (representative == nullptr && renderedColor != 0) representative = color;
        if (pixels != nullptr && renderedColor != pixels->getPixelColor(i)
            && (!colorLightList.exist(i) || !colorLightList.get(i)->isOn())) {
            pixels->setPixelColor(i,
                (renderedColor >> 16) & 255,
                (renderedColor >> 8) & 255,
                renderedColor & 255);
        }
    }
    // In an intentionally dark effect frame, retain the selected light color.
    // When the light is off its current value also fades to zero as before.
    statusLed.setEffectColor(representative ? *representative : statusLed.getCurrentColor());
}

inline void createPixels()
{
    if (ledsPin >= 0) {
        pixels = new Adafruit_NeoPixel(ledsConfig.count, ledsPin, pixelsType + pixelsSpeed);
        // turn off
        //pixels->clear();
        //*
        for (int i = 0; i < ledsConfig.count; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }

        setupPixelModules();

    }
}

inline void disposePixels() {

    // turn off all pixels
    if (pixels != nullptr) {
        //pixels->clear();
        //*
        for (int i = 0; i < ledsConfig.count; i++) {
            pixels->setPixelColor(i, 0, 0, 0);
        }
        refresh();
        //*/
        delete pixels;
        pixels = nullptr;
    }

}

// Called only between frames, never from the option/API callback.
inline void applyPendingLedCount() {
    static uint16_t lastRequest = 0;
    const uint16_t count = requestedLedCount.load();
    if (count == 0 || count == lastRequest) return;
    lastRequest = count;
    if (count == ledsConfig.count) return;

    Adafruit_NeoPixel* replacement = nullptr;
    if (ledsPin >= 0) {
        replacement = new (std::nothrow) Adafruit_NeoPixel(count, ledsPin, pixelsType + pixelsSpeed);
        if (replacement != nullptr && replacement->numPixels() != count) {
            delete replacement;
            replacement = nullptr;
        }
    }
    if ((ledsPin >= 0 && replacement == nullptr) || !fx_resize(count, statusLed.getCurrentColor())) {
        delete replacement;
        // NeoPixel destruction releases its GPIO; restore the retained output.
        if (pixels != nullptr) pixels->begin();
        IO::Logger::error("|  x Not enough memory to resize LED output; keeping %u LEDs", ledsConfig.count);
        auto option = homeGenie->getDefaultModule()->getProperty(Options::Controller::LED_count);
        if (option != nullptr) option->setValue(String(ledsConfig.count).c_str());
        return;
    }
    // Clear removed LEDs before dropping the old hardware buffer.
    disposePixels();
    pixels = replacement;
    ledsConfig.count = count;
    ledsConfig.cols = ledsConfig.rows > 0 ? count / ledsConfig.rows : count;
    if (pixels != nullptr) pixels->begin();
    setupPixelModules();
#ifndef DISABLE_AUTOMATION
    // ALL resolves at playback time; explicit ranges are clipped by the player.
    if (scriptHandler != nullptr) scriptHandler->startTime = millis();
#endif
    styleChanged = true;
    levelChanged = true;
}

// Adds 2 "GpioButton" modules for handling manual control (on/off, level, switch fx, ...)
inline void setupControlButtons(Module* miniModule) {
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
        auto button2 = new Button(IO::IOEventDomains::HomeAutomation_HomeGenie, "B2", "Button 2", btn2_pin);
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
                    levelChanged = true;
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
                        auto msg = std::make_shared<QueuedMessage>(miniModule, "FX.Style", lightStyleNames[currentStyleIndex]);
                        miniModule->setProperty("FX.Style", lightStyleNames[currentStyleIndex]);
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
inline void setupDefaultSchedules() {

    // Create default Dawn scene
    if (Scheduler::get("Dawn") == nullptr) {
        auto s = new Schedule("Dawn", "Dawn scene colors, 24 minutes before sunrise.", "", "@SolarTimes.Sunrise - 24", "$$.boundModules.on();");
        s->onModuleEvent = false;
        //s->eventModules.add(miniModule->getReference());
        // UI state data
        s->data = R"({"action":{"template":{"forEach":{"config":{"color":"#FF9100|30.0"},"enabled":true,"id":"command_set_color"},"forEnd":{"config":{},"enabled":false,"id":null},"forStart":{"config":{},"enabled":false,"id":null}},"type":"template"},"event":[],"from":"","itemType":3,"occur_dayom_sel":[],"occur_dayom_type":1,"occur_dayow_sel":[],"occur_hour_sel":[],"occur_hour_step":12,"occur_hour_type":1,"occur_min_sel":[],"occur_min_step":30,"occur_min_type":1,"occur_month_sel":[],"occur_month_type":1,"time":[],"to":""})";
        // Device types allowed
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
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
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
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
        s->boundDevices.add(new String(ModuleType::Light));
        s->boundDevices.add(new String(ModuleType::Dimmer));
        s->boundDevices.add(new String(ModuleType::Color));
        Scheduler::addSchedule(s);
    }

    if (true || Scheduler::getScheduleList().size() == 0) {

        // TODO: if added at least one schedule then  ->  Scheduler::save();

    }

}
#endif // DISABLE_AUTOMATION


// UI options update listener
static class : public ModuleParameter::UpdateListener {
public:
    void onUpdate(ModuleParameter* option) override {

        if (option->is(Options::Light::FX_Style)) {

            // Update current rendering style
            auto style = getLightStyleIndex(option->value);
            if (style != currentStyleIndex) currentStyleIndex = style;

        } else if (option->is(Options::Light::FX_Strobe)) {

            // enable / disable strobe light
            if (strobeFxTickMs > 0 && option->value == "off") {
                strobeFxTickMs = 0;
            } else if (strobeFxTickMs == 0 && option->value != "off") {
                strobeFxTickMs = millis();
            }

        } else if (option->is(Options::Controller::LED_count)) {

            const long count = option->value.toInt();
            requestedLedCount.store(static_cast<uint16_t>(count < 1 ? 1 : count > 1920 ? 1920 : count));

        } else if (option->is(Options::Controller::LED_rows)) {

            int rows = option->value.toInt();
            ledsConfig.rows = max(1, rows);
            ledsConfig.cols = ledsConfig.count / ledsConfig.rows;

        } else if (option->is(Options::Controller::LED_vert)) {

            ledsConfig.isVertical = (option->value.equalsIgnoreCase("true"));

        } else if (option->is(Options::Controller::LED_serp)) {

            ledsConfig.isSerpentine = (option->value.equalsIgnoreCase("true"));

        } else if (option->is(Options::Controller::LED_stbt)) {

            ledsConfig.startAtBottom = (option->value.equalsIgnoreCase("true"));

        } else if (option->is(Options::Controller::LED_strg)) {

            ledsConfig.startAtRight = (option->value.equalsIgnoreCase("true"));

        } else if (option->is(Options::Controller::LED_power)) {

            // max power value changed, update it
            maxPower = option->value.toInt();

        }

    }

} optionUpdateListener;

inline void setupControllerOptions(Module* miniModule) {
    // Number select control to set the number of LEDs of the strip
    miniModule->addWidgetOption(
            // name, value
            Options::Controller::LED_count, String(ledsConfig.count).c_str(),
            // type
            UI_WIDGETS_FIELD_TYPE_NUMBER
            // label
            ":led_count"
            // min:max:default
            ":1:1920:1"
    )->withConfigKey(Configuration::LedsCount)->addUpdateListener(&optionUpdateListener);
    // Number of rows if it's a LEDs matrix
    miniModule->addWidgetOption(
        // name, value
        Options::Controller::LED_rows, String(ledsConfig.rows).c_str(),
        // type
        UI_WIDGETS_FIELD_TYPE_NUMBER
        // label
        ":led_rows"
        // min:max:default
        ":1:256:1"
    )->withConfigKey(Configuration::LedsRows)->addUpdateListener(&optionUpdateListener);
    // Is Vertical Matrix
    miniModule->addWidgetOption(
        // name, value
        Options::Controller::LED_vert, (ledsConfig.isVertical ? "1" : "0"),
        // type
        UI_WIDGETS_FIELD_TYPE_CHECKBOX
        // label
        ":is_vertical_matrix"
        // default
        ""
    )->withConfigKey(Configuration::LedsIsVert)->addUpdateListener(&optionUpdateListener);
    // Is Serpentine Matrix
    miniModule->addWidgetOption(
        // name, value
        Options::Controller::LED_serp, (ledsConfig.isSerpentine ? "1" : "0"),
        // type
        UI_WIDGETS_FIELD_TYPE_CHECKBOX
        // label
        ":is_serpentine_matrix"
        // default
        ""
    )->withConfigKey(Configuration::LedsIsSerp)->addUpdateListener(&optionUpdateListener);
    // Matrix starts at bottom
    miniModule->addWidgetOption(
        // name, value
        Options::Controller::LED_stbt, (ledsConfig.startAtBottom ? "1" : "0"),
        // type
        UI_WIDGETS_FIELD_TYPE_CHECKBOX
        // label
        ":matrix_starts_at_bottom"
        // default
        ""
    )->withConfigKey(Configuration::LedsStBottom)->addUpdateListener(&optionUpdateListener);
    // Matrix starts at right
    miniModule->addWidgetOption(
        // name, value
        Options::Controller::LED_strg, (ledsConfig.startAtRight ? "1" : "0"),
        // type
        UI_WIDGETS_FIELD_TYPE_CHECKBOX
        // label
        ":matrix_starts_at_right"
        // default
        ""
    )->withConfigKey(Configuration::LedsStRight)->addUpdateListener(&optionUpdateListener);
    // Number select control to set maximum percentage of power that can be drawn
    miniModule->addWidgetOption(
            // name, value
            Options::Controller::LED_power, String(maxPower).c_str(),
            // type
            UI_WIDGETS_FIELD_TYPE_NUMBER
            // label
            ":led_power"
            // min:max:default
            ":5:100:25"
    )->withConfigKey(Configuration::LedsPower)->addUpdateListener(&optionUpdateListener);
}

inline void setupColorLightOptions(Module* module) {
    // build fx list
    String selectConfig = String(UI_WIDGETS_FIELD_TYPE_SELECT) + ":light_style";
    for (int i = 0; i < STYLES_COUNT; i++) {
        selectConfig += (i == 0 ? ":" : "|");
        selectConfig += lightStyleNames[i];
    }
    // - dropdown list control to select the light animation effect
    module->addWidgetOption(
            Options::Light::FX_Style,
            lightStyleNames[currentStyleIndex],
            selectConfig.c_str()
    )->addUpdateListener(&optionUpdateListener);
    // - dropdown list control to enable the strobe effect
    mpFxStrobe = module->addWidgetOption(
            // name, value
            Options::Light::FX_Strobe, "off",
            // type
            UI_WIDGETS_FIELD_TYPE_SELECT
            // label
            ":strobe_effect"
            // options
            ":off"
            "|slow"
            "|medium"
            "|fast"
    );
    mpFxStrobe->addUpdateListener(&optionUpdateListener);
}
