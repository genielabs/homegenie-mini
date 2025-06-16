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

#include "LevelControlActivity.h"

#ifdef ENABLE_UI
#ifndef DISABLE_LVGL

namespace UI { namespace Activities { namespace Control {

    LevelControlActivity::LevelControlActivity(const char *moduleAddress) {
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

    bool LevelControlActivity::canHandleDomain(String *domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool LevelControlActivity::handleRequest(APIRequest *request, ResponseCallback *callback) {
        return false;
    }

    bool LevelControlActivity::handleEvent(IIOEventSender *, const char *domain, const char *address,
                                           const char *eventPath, void *eventData, IOEventDataType) {
        return false;
    }

    Module *LevelControlActivity::getModule(const char *domain, const char *address) {
        if (module.domain.equals(domain) && module.address.equals(address))
            return &module;
        return nullptr;
    }

    LinkedList<Module *> *LevelControlActivity::getModuleList() {
        return &moduleList;
    }

    void LevelControlActivity::onResume() {
        LvglActivity::onResume();

        auto w = (float)canvas->width();
        auto h = (float)canvas->height();
        auto diameterRatio = 1.8f;
        diameter = (uint16_t)round(max(w, h) / diameterRatio);
        auto indicatorSize = (uint8_t)round((float)diameter / 7.0f);
        auto knobSize = (uint8_t)round(indicatorSize * diameterRatio);
        auto toggleSize = 56;
        auto shiftY = diameter / 10;

        // Background gradient
        if (!backgroundGradient) {
            backgroundGradient = new lv_style_t();
            lv_style_init(backgroundGradient);
            lv_style_set_radius(backgroundGradient, 0);
            lv_style_set_border_width(backgroundGradient, 0);
            lv_style_set_bg_opa(backgroundGradient, LV_OPA_COVER);
            lv_style_set_bg_grad_dir(backgroundGradient, LV_GRAD_DIR_VER);
            lv_style_set_bg_color(backgroundGradient, lv_color_make(30, 0, 0));
            lv_style_set_bg_grad_color(backgroundGradient, lv_color_make(0, 0, 80));
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

        // Level slider
        if (!circularSlider) {
            circularSlider = lv_arc_create(activeScreen);
            lv_obj_set_size(circularSlider, diameter, diameter);
            lv_obj_align(circularSlider, LV_ALIGN_CENTER, 0, shiftY);
            lv_arc_set_range(circularSlider, 0, 100);
            lv_arc_set_rotation(circularSlider, 150);
            lv_arc_set_bg_angles(circularSlider, 0, 240);
            lv_obj_set_style_arc_width(circularSlider, indicatorSize, LV_PART_MAIN);
            lv_obj_set_style_arc_width(circularSlider, indicatorSize, LV_PART_INDICATOR);
            lv_obj_set_style_radius(circularSlider, knobSize, LV_PART_KNOB);
            lv_obj_set_style_bg_color(circularSlider, lv_color_white(), LV_PART_KNOB);
            lv_obj_set_style_border_width(circularSlider, 1, LV_PART_KNOB);
            lv_obj_set_style_border_color(circularSlider, lv_color_black(), LV_PART_KNOB);
            lv_obj_add_event_cb(circularSlider, circularSliderEventHandler, LV_EVENT_VALUE_CHANGED, this);
            activityEventsLock(circularSlider, this);
        }

        // Toggle button
        if (!toggleButton) {
            toggleButton = lv_btn_create(activeScreen);
            lv_obj_set_size(toggleButton, toggleSize, toggleSize);
            lv_obj_align(toggleButton, LV_ALIGN_CENTER, 0, shiftY);
            lv_obj_set_style_radius(toggleButton, LV_RADIUS_CIRCLE, LV_PART_MAIN);
            lv_obj_set_style_bg_opa(toggleButton, LV_OPA_COVER, LV_PART_MAIN);
            lv_obj_add_event_cb(toggleButton, toggleButtonEventHandler, LV_EVENT_CLICKED, this);
            activityEventsLock(toggleButton, this);
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

        // Level label
        if (!labelLevel) {
            labelLevel = lv_label_create(activeScreen);
        }

        refreshData();
    }

    void LevelControlActivity::onPause() {
        LvglActivity::onPause();

        //lv_style_reset(backgroundGradient);
        backgroundGradient = nullptr;

        labelTitle = nullptr;
        labelLevel = nullptr;
        circularSlider = nullptr;
        toggleButton = nullptr;
        ledStatus = nullptr;
    }

    void LevelControlActivity::refreshData() {
        // Update level
        lv_arc_set_value(circularSlider, data.level);
        // Update label
        char buf[8];
        lv_snprintf(buf, sizeof(buf), "%d%%", data.level);
        lv_label_set_text(labelLevel, data.isOn ? buf : "OFF");
        lv_obj_align_to(labelLevel, circularSlider, LV_ALIGN_CENTER, 0, diameter / 2.8f);
        // Toggle button color
        auto l = ((float)data.level / 100.0f) * 255.0f;
        auto rgb = lv_color_make(l, l, l / 1.25f);
        lv_obj_set_style_bg_color(toggleButton, rgb, 0);
        // Status LED
        if (data.isOn) {
            lv_led_on(ledStatus);
        } else {
            lv_led_off(ledStatus);
        }
    }

    void LevelControlActivity::emitLevelEvent() {
        // Emit event
        auto l = data.level / 100.0f;
        auto level = data.isOn ? String(l, 3) : String("0");
        module.setProperty(IOEventPaths::Sensor_Level, level);
        auto me = std::make_shared<QueuedMessage>(&module, IOEventPaths::Sensor_Level, level);
        HomeGenie::getInstance()->getEventRouter().signalEvent(me);
    }

    void LevelControlActivity::activityEventsLock(lv_obj_t *obj, void *instance) {
        lv_obj_add_event_cb(obj, uiLockEventHandler, LV_EVENT_PRESSED, instance);
        lv_obj_add_event_cb(obj, uiLockEventHandler, LV_EVENT_RELEASED, instance);
    }

    void LevelControlActivity::uiLockEventHandler(lv_event_t *e) {
        auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
        if (instance) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_PRESSED) {
                instance->lock();
            } else if (code == LV_EVENT_RELEASED) {
                instance->unlock();
            }
        }
    }

    void LevelControlActivity::toggleButtonEventHandler(lv_event_t *e) {
        auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
        if (instance) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_CLICKED) {
                instance->data.isOn = !instance->data.isOn;
                instance->emitLevelEvent();
                instance->refreshData();
            }
        }
    }

    void LevelControlActivity::circularSliderEventHandler(lv_event_t *e) {
        auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
        if (instance && instance->circularSlider) {
            auto code = lv_event_get_code(e);
            if (code == LV_EVENT_VALUE_CHANGED) {
                auto v = lv_arc_get_value(instance->circularSlider);
                instance->data.level = v;
                instance->data.isOn = (v > 0);
                instance->emitLevelEvent();
                instance->refreshData();
            }
        }
    }

}}}

#endif // DISABLE_LVGL
#endif // ENABLE_UI
