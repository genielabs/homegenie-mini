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

#include "ColorControlActivity.h"

#ifdef ENABLE_UI
#ifndef DISABLE_LVGL

namespace UI { namespace Activities { namespace Control {

    ColorControlActivity::ColorControlActivity(const char *moduleAddress) {
        module.domain  = IO::IOEventDomains::HomeAutomation_HomeGenie;
        module.address = moduleAddress;
        module.type    = ModuleType::Sensor;
        // load stored name for this module
        auto key = String(CONFIG_KEY_ACTIVITY_TITLE); key.concat(moduleAddress);
        module.name = Config::getSetting(key.c_str(), moduleAddress);
        module.setProperty(Implements::Scheduling, "true");
        module.setProperty(Implements::Scheduling_ModuleEvents, "true");
        module.onNameUpdated = [this](const char* oldName, const char* newName) {
            if (labelTitle) {
                lv_label_set_text(labelTitle, newName);
            }
            auto key = String(CONFIG_KEY_ACTIVITY_TITLE);
            key.concat(module.address);
            Config::saveSetting(key.c_str(), newName);
        };
        moduleList.add(&module);
        HomeGenie::getInstance()->addAPIHandler(this);
        LV_FONT_DECLARE(material_symbols_36);
    }

    void ColorControlActivity::init() {
    }

    bool ColorControlActivity::canHandleDomain(String *domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool ColorControlActivity::handleRequest(APIRequest *request, ResponseCallback *callback) {
        return false;
    }

    bool ColorControlActivity::handleEvent(IIOEventSender *, const char *domain, const char *address,
                                           const char *eventPath, void *eventData, IOEventDataType) {
        return false;
    }

    Module *ColorControlActivity::getModule(const char *domain, const char *address) {
        if (module.domain.equals(domain) && module.address.equals(address))
            return &module;
        return nullptr;
    }

    LinkedList<Module *> *ColorControlActivity::getModuleList() {
        return &moduleList;
    }

    void ColorControlActivity::onResume() {
        LvglActivity::onResume();

        auto w = (float)canvas->width();
        auto h = (float)canvas->height();
        auto diameterRatio = 1.8f;
        diameter = (uint16_t)round(h / diameterRatio);
        auto indicatorSize = (uint8_t)round((float)diameter / 7.0f);
        auto knobSize = (uint8_t)round(indicatorSize * diameterRatio);
        auto toggleSize = 56;
        shiftY = diameter / 10;

        // Background gradient
        if (!backgroundGradient) {
            backgroundGradient = new lv_style_t();
            lv_style_init(backgroundGradient);
            lv_style_set_radius(backgroundGradient, 0);
            lv_style_set_border_width(backgroundGradient, 0);
            lv_style_set_bg_opa(backgroundGradient, LV_OPA_COVER);
            lv_style_set_bg_grad_dir(backgroundGradient, LV_GRAD_DIR_VER);
            lv_style_set_bg_color(backgroundGradient, lv_color_make(80, 0, 0));
            lv_style_set_bg_grad_color(backgroundGradient, lv_color_make(30, 30, 0));
            lv_obj_add_style(activeScreen, backgroundGradient, 0);
        }

        // Module name
        if (!labelTitle) {
            static lv_anim_t animation_template;
            static lv_style_t label_style;
            lv_anim_init(&animation_template);
            lv_anim_set_delay(&animation_template, 1000);           //Wait 1 second to start the first scroll
            lv_anim_set_repeat_delay(&animation_template, 3000);    //Repeat the scroll 3 seconds after the label scrolls back to the initial position
            //Initialize the label style with the animation template
            lv_style_init(&label_style);
            lv_style_set_anim(&label_style, &animation_template);
            labelTitle = lv_label_create(activeScreen);
            lv_label_set_long_mode(labelTitle, LV_LABEL_LONG_SCROLL_CIRCULAR);      //Circular scroll
            lv_obj_set_style_text_font(labelTitle, &lv_font_montserrat_24, 0);
            lv_label_set_text(labelTitle, module.name.c_str());
            lv_obj_add_style(labelTitle, &label_style, LV_STATE_DEFAULT);
            lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 10);
        }

        // Color wheel
        if (!colorWheel) {
            colorWheel = lv_colorwheel_create(activeScreen, false);
            lv_obj_set_size(colorWheel, diameter, diameter);
            lv_obj_align(colorWheel, LV_ALIGN_CENTER, 0, -8);
            //lv_arc_set_rotation(colorWheel, 150);
            //lv_arc_set_range(colorWheel, 0, 240);
            lv_obj_set_style_arc_width(colorWheel, indicatorSize, LV_PART_MAIN);
            lv_obj_set_style_arc_width(colorWheel, knobSize, LV_PART_KNOB);
            lv_obj_set_style_pad_all(colorWheel, 3, LV_PART_KNOB);
            lv_obj_set_style_bg_color(colorWheel, lv_color_white(), LV_PART_KNOB);
            lv_obj_set_style_border_width(colorWheel, 1, LV_PART_KNOB);
            lv_obj_set_style_border_color(colorWheel, lv_color_black(), LV_PART_KNOB);
            lv_obj_add_event_cb(colorWheel, colorWheelEventHandler, LV_EVENT_VALUE_CHANGED, this);
            activityEventsLock(colorWheel, this);
        }

        // Saturation/Brightness wheel
        if (!circularSlider) {
            circularSlider = lv_arc_create(activeScreen);
            lv_obj_set_size(circularSlider, diameter, diameter);
            lv_obj_align(circularSlider, LV_ALIGN_CENTER, 0, -8);
            lv_arc_set_range(circularSlider, 0, 100);
            lv_arc_set_rotation(circularSlider, 150);
            lv_arc_set_bg_angles(circularSlider, 0, 240);
            lv_obj_set_style_arc_width(circularSlider, indicatorSize, LV_PART_MAIN);
            lv_obj_set_style_arc_width(circularSlider, indicatorSize, LV_PART_INDICATOR);
            lv_obj_set_style_radius(circularSlider, knobSize, LV_PART_KNOB);
            lv_obj_set_style_bg_color(circularSlider, lv_color_white(), LV_PART_KNOB);
            lv_obj_set_style_border_width(circularSlider, 1, LV_PART_KNOB);
            lv_obj_set_style_border_color(circularSlider, lv_color_black(), LV_PART_KNOB);

            /*
            value_label = lv_label_create(parent_screen);
            lv_label_set_text_fmt(value_label, "Valore: %d", lv_arc_get_value(circularSlider));
            lv_obj_align_to(value_label, circularSlider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
            */

            lv_obj_add_event_cb(circularSlider, circularSliderEventHandler, LV_EVENT_VALUE_CHANGED, this);
            activityEventsLock(circularSlider, this);
            lv_obj_add_flag(circularSlider, LV_OBJ_FLAG_HIDDEN);
        }

        // Toggle button
        if (!toggleButton) {
            toggleButton = lv_btn_create(activeScreen);
            initRoundButton(toggleButton);
            lv_obj_set_size(toggleButton, toggleSize, toggleSize);
            lv_obj_align(toggleButton, LV_ALIGN_CENTER, 0, -8);
            lv_obj_add_event_cb(toggleButton, toggleButtonEventHandler, LV_EVENT_CLICKED, this);
        }

        // Status LED
        if (!ledStatus) {
            ledStatus = lv_led_create(toggleButton);
            lv_obj_align(ledStatus, LV_ALIGN_CENTER, 0, 0);
            lv_led_set_color(ledStatus, lv_palette_main(LV_PALETTE_LIME));
            lv_obj_set_size(ledStatus, 25, 25);
            lv_obj_set_style_border_width(ledStatus, 4, LV_PART_MAIN);
            lv_obj_set_style_border_color(ledStatus, lv_color_black(), LV_PART_MAIN);
            lv_obj_clear_flag(ledStatus, LV_OBJ_FLAG_CLICKABLE);
        }

        // Mode select buttons
        auto buttonShiftX = (uint16_t)round((float)w / 3.6f);
        if (!buttonColor) {
            buttonColor = lv_btn_create(activeScreen);
            lv_obj_add_flag(buttonColor, LV_OBJ_FLAG_CHECKABLE);
            initRoundButton(buttonColor);
            addButtonIcon(buttonColor, "\uE40A"); // Palette symbol
            lv_obj_align(buttonColor, LV_ALIGN_BOTTOM_MID, -buttonShiftX, isRoundDisplay() ? -28 : -1);
            lv_obj_add_event_cb(buttonColor, modeButtonEventHandler, LV_EVENT_CLICKED, this);
        }
        if (!buttonSaturation) {
            buttonSaturation = lv_btn_create(activeScreen);
            lv_obj_add_flag(buttonSaturation, LV_OBJ_FLAG_CHECKABLE);
            initRoundButton(buttonSaturation);
            addButtonIcon(buttonSaturation, "\uE91C"); // Saturation symbol
            lv_obj_align(buttonSaturation, LV_ALIGN_BOTTOM_MID, 0, -1);
            lv_obj_add_event_cb(buttonSaturation, modeButtonEventHandler, LV_EVENT_CLICKED, this);
        }
        if (!buttonValue) {
            buttonValue = lv_btn_create(activeScreen);
            lv_obj_add_flag(buttonValue, LV_OBJ_FLAG_CHECKABLE);
            initRoundButton(buttonValue);
            addButtonIcon(buttonValue, "\uE3A9"); // Brightness symbol
            lv_obj_align(buttonValue, LV_ALIGN_BOTTOM_MID, buttonShiftX, isRoundDisplay() ? -28 : -1);
            lv_obj_add_event_cb(buttonValue, modeButtonEventHandler, LV_EVENT_CLICKED, this);
        }

        // Level label
        if (!labelLevel) {
            labelLevel = lv_label_create(activeScreen);
        }

        refreshData();
    }

    void ColorControlActivity::onPause() {
        LvglActivity::onPause();

        //lv_style_reset(backgroundGradient);
        backgroundGradient = nullptr;

        labelTitle = nullptr;
        labelLevel = nullptr;
        colorWheel = nullptr;
        circularSlider = nullptr;
        toggleButton = nullptr;
        ledStatus = nullptr;
        buttonColor = nullptr;
        buttonSaturation = nullptr;
        buttonValue = nullptr;
    }

    void ColorControlActivity::refreshData() {
        if (!colorWheel || !circularSlider || !toggleButton || !buttonColor || !buttonSaturation || !buttonValue || !ledStatus) {
            return;
        }
        char buf[8]; // used for status label
        lv_obj_clear_state(buttonColor, LV_STATE_CHECKED);
        lv_obj_clear_state(buttonSaturation, LV_STATE_CHECKED);
        lv_obj_clear_state(buttonValue, LV_STATE_CHECKED);
        switch (data.controlMode) {
            case EDIT_HUE: {
                lv_obj_clear_flag(colorWheel, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(circularSlider, LV_OBJ_FLAG_HIDDEN);
                lv_color_hsv_t hsv;
                hsv.h = data.hue;
                hsv.s = 100; //data.saturation;
                hsv.v = 100; //data.value;
                lv_colorwheel_set_hsv(colorWheel, hsv);
                lv_obj_add_state(buttonColor, LV_STATE_CHECKED);
                // label
                lv_label_set_text(labelLevel, data.isOn ? "ON" : "OFF");
                lv_obj_align_to(labelLevel, colorWheel, LV_ALIGN_CENTER, 0, 28 + round(diameter / 14));
            } break;
            case EDIT_SATURATION: {
                lv_obj_add_flag(colorWheel, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(circularSlider, LV_OBJ_FLAG_HIDDEN);
                lv_arc_set_value(circularSlider, data.saturation);
                lv_obj_add_state(buttonSaturation, LV_STATE_CHECKED);
                // label
                lv_snprintf(buf, sizeof(buf), "%d%%", data.saturation);
                lv_label_set_text(labelLevel, data.isOn ? buf : "OFF");
                lv_obj_align_to(labelLevel, circularSlider, LV_ALIGN_CENTER, 0, diameter / 2.8f);
            } break;
            case EDIT_VALUE: {
                lv_obj_add_flag(colorWheel, LV_OBJ_FLAG_HIDDEN);
                lv_obj_clear_flag(circularSlider, LV_OBJ_FLAG_HIDDEN);
                lv_arc_set_value(circularSlider, data.value);
                lv_obj_add_state(buttonValue, LV_STATE_CHECKED);
                // label
                lv_snprintf(buf, sizeof(buf), "%d%%", data.value);
                lv_label_set_text(labelLevel, data.isOn ? buf : "OFF");
                lv_obj_align_to(labelLevel, circularSlider, LV_ALIGN_CENTER, 0, diameter / 2.8f);
            } break;
        }
        // Current color
        auto rgb = lv_color_hsv_to_rgb(data.hue, data.saturation, data.value);
        lv_obj_set_style_bg_color(toggleButton, rgb, 0);
        // Status LED
        if (data.isOn) {
            lv_led_on(ledStatus);
        } else {
            lv_led_off(ledStatus);
        }
    }

    void ColorControlActivity::emitHsvEvent() {
        lv_color_hsv_t hsv;
        hsv.h = data.hue;
        hsv.s = data.saturation;
        hsv.v = data.value;
        // Emit event
        auto color = hsvToNormalizedString(hsv);
        module.setProperty(IOEventPaths::Sensor_ColorHsv, color);
        auto me = QueuedMessage(&module, IOEventPaths::Sensor_ColorHsv, color);
        HomeGenie::getInstance()->getEventRouter().signalEvent(me);
    }

    void ColorControlActivity::emitLevelEvent() {
        // Emit event
        auto l = data.value / 100.0f;
        auto level = data.isOn ? String(l, 3) : String("0");
        module.setProperty(IOEventPaths::Sensor_Level, level);
        auto me = QueuedMessage(&module, IOEventPaths::Sensor_Level, level);
        HomeGenie::getInstance()->getEventRouter().signalEvent(me);
    }

    void ColorControlActivity::activityEventsLock(lv_obj_t *obj, void *instance) {
        lv_obj_add_event_cb(obj, uiLockEventHandler, LV_EVENT_PRESSED, instance);
        lv_obj_add_event_cb(obj, uiLockEventHandler, LV_EVENT_RELEASED, instance);
    }

    void ColorControlActivity::uiLockEventHandler(lv_event_t *e) {
        auto instance = static_cast<ColorControlActivity*>(lv_event_get_user_data(e));
        if (instance) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_PRESSED) {
                instance->lock();
            } else if (code == LV_EVENT_RELEASED) {
                instance->unlock();
            }
        }
    }

    void ColorControlActivity::modeButtonEventHandler(lv_event_t *e) {
        auto target = lv_event_get_target(e);
        auto instance = static_cast<ColorControlActivity*>(lv_event_get_user_data(e));
        if (instance && target) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_CLICKED) {
                if (target == instance->buttonColor) {
                    instance->data.controlMode = EDIT_HUE;
                } else if (target == instance->buttonSaturation) {
                    instance->data.controlMode = EDIT_SATURATION;
                } else if (target == instance->buttonValue) {
                    instance->data.controlMode = EDIT_VALUE;
                }
                instance->refreshData();
            }
        }
    }

    void ColorControlActivity::toggleButtonEventHandler(lv_event_t *e) {
        auto instance = static_cast<ColorControlActivity*>(lv_event_get_user_data(e));
        if (instance) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_CLICKED) {
                instance->data.isOn = !instance->data.isOn;
                instance->emitLevelEvent();
                instance->refreshData();
            }
        }
    }

    void ColorControlActivity::colorWheelEventHandler(lv_event_t *e) {
        auto instance = static_cast<ColorControlActivity*>(lv_event_get_user_data(e));
        if (instance && instance->colorWheel) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_VALUE_CHANGED) {
                auto hsv = lv_colorwheel_get_hsv(instance->colorWheel);
                instance->data.hue = hsv.h;
                if (instance->data.value == 0) {
                    instance->data.value = 100;
                }
                instance->data.isOn = true;
                instance->emitHsvEvent();
                instance->refreshData();
            }
        }
    }

    void ColorControlActivity::circularSliderEventHandler(lv_event_t *e) {
        auto instance = static_cast<ColorControlActivity*>(lv_event_get_user_data(e));
        if (instance && instance->circularSlider) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_VALUE_CHANGED) {
                auto v = lv_arc_get_value(instance->circularSlider);
                if (instance->data.controlMode == EDIT_SATURATION) {
                    instance->data.saturation = v;
                    if (instance->data.value == 0) {
                        instance->data.value = 100;
                    }
                    instance->data.isOn = true;
                    instance->emitHsvEvent();
                } else  {
                    instance->data.value = v;
                    instance->data.isOn = (v > 0);
                    instance->emitLevelEvent();
                }
                instance->refreshData();
            }
        }
    }

    String ColorControlActivity::hsvToNormalizedString(const lv_color_hsv_t &hsv_color) {
        float h_norm = static_cast<float>(hsv_color.h) / 359.0f;
        float s_norm = static_cast<float>(hsv_color.s) / 100.0f;
        float v_norm = static_cast<float>(hsv_color.v) / 100.0f;
        String result = String(h_norm, 8);
        result += ",";
        result += String(s_norm, 3);
        result += ",";
        result += String(v_norm, 3);
        return result;
    }

    void ColorControlActivity::initRoundButton(lv_obj_t *button) {
        lv_obj_set_size(button, 48, 48);
        lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        lv_obj_set_style_bg_color(button, lv_palette_main(LV_PALETTE_NONE), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_PART_MAIN);
        activityEventsLock(button, this);
    }

    void ColorControlActivity::addButtonIcon(lv_obj_t *button, const char *symbol) {
        lv_obj_t* icon = lv_label_create(button);
        lv_obj_set_style_text_font(icon, &material_symbols_36, 0);
        lv_label_set_text(icon, symbol); // Show next mode icon (Saturation)
        lv_obj_set_style_text_color(icon, lv_color_white(), 0);
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);
    }

}}}

#endif // DISABLE_LVGL
#endif // ENABLE_UI
