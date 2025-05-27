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

#ifndef HOMEGENIE_MINI_LEVELCONTROLACTIVITY_H
#define HOMEGENIE_MINI_LEVELCONTROLACTIVITY_H

#include "Common.h"

#ifdef ENABLE_UI

#include "service/api/APIRequest.h"
#include "service/api/devices/Dimmer.h"
#include "ui/fonts/material_symbols_common.h"

#ifndef DISABLE_LVGL

#include "ui/LvglDriver.h"

namespace UI { namespace Activities { namespace Control {

    using namespace Service::API;
    using namespace Service::API::devices;
    using namespace Service::ModuleApi;
    using namespace Service::WidgetApi;

    class DimmerControlData {
    public:
        int level = 50;
    };

    class LevelControlActivity : public Activity, public APIHandler {
    public:
        LinkedList<Module*> moduleList;
        Module module;

        explicit LevelControlActivity(const char* moduleAddress) {
            setDrawInterval(5); // Task.h
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
        }

        void attach(LGFX_Device *displayDevice) override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void init() override {
        }
        bool canHandleDomain(String* domain) override {
            return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
        }
        bool handleRequest(APIRequest* request, ResponseCallback* callback) override {
            return false;
        }
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override {
            return false;
        }
        Module* getModule(const char* domain, const char* address) override {
            if (module.domain.equals(domain) && module.address.equals(address))
                return &module;
            return nullptr;
        }
        LinkedList<Module*>* getModuleList() override {
            return &moduleList;
        }

    private:
        lv_obj_t* activeScreen = nullptr;
        lv_obj_t* sliderLevel = nullptr;
        lv_obj_t* labelLevel = nullptr;
        lv_obj_t* buttonOn = nullptr;
        lv_obj_t* labelOn = nullptr;
        lv_obj_t* buttonOff = nullptr;
        lv_obj_t* labelOff = nullptr;
        lv_obj_t* ledStatus = nullptr;
        lv_obj_t* labelTitle = nullptr;
        lv_obj_t* iconDeviceType = nullptr;

        DimmerControlData data;

        static void applyCommonStyles(lv_obj_t* obj) {
            lv_obj_set_style_border_width(obj, 1, 0);
            lv_obj_set_style_border_color(obj, lv_color_black(), 0);
            lv_obj_set_style_border_opa(obj, LV_OPA_COVER, 0);
        }
        static void activityEventsLock(lv_obj_t* obj, void* instance) {
            lv_obj_add_event_cb(obj, ui_lock_event_handler, LV_EVENT_PRESSED, instance);
            lv_obj_add_event_cb(obj, ui_lock_event_handler, LV_EVENT_RELEASED, instance);
        }
        static void sliderEventHandler(lv_event_t * e)
        {
            auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
            if (instance) {
                lv_event_code_t code = lv_event_get_code(e);
                if (code == LV_EVENT_VALUE_CHANGED) {
                    instance->data.level = lv_slider_get_value(instance->sliderLevel);
                    char buf[8];
                    lv_snprintf(buf, sizeof(buf), "%d%%", instance->data.level);
                    lv_label_set_text(instance->labelLevel, buf);
                    // Emit event
                    auto module = &instance->module;
                    auto l = instance->data.level / 100.0f;
                    auto level = String(l, 3);
                    module->setProperty(EVENT_SENSOR_LEVEL, level);
                    auto me = QueuedMessage(module, EVENT_SENSOR_LEVEL, level, &l, IO::IOEventDataType::Number);
                    HomeGenie::getInstance()->getEventRouter().signalEvent(me);
                    // update LED
                    if (instance->ledStatus) {
                        if (l > 0) {
                            lv_led_on(instance->ledStatus);
                        } else {
                            lv_led_off(instance->ledStatus);
                        }
                    }
                }
            }
        }
        static void buttonEventHandler(lv_event_t * e)
        {
            auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
            if (instance) {
                lv_event_code_t code = lv_event_get_code(e);
                if (code == LV_EVENT_CLICKED) {
                    // Emit event
                    lv_obj_t *b = lv_event_get_target(e);
                    if (b) {
                        auto value = (b == instance->buttonOn) ? "on" : "off";
                        auto module = &instance->module;
                        module->setProperty(EVENT_SENSOR_BUTTON, value);
                        auto me = QueuedMessage(module, EVENT_SENSOR_BUTTON, value, nullptr, IO::IOEventDataType::Text);
                        HomeGenie::getInstance()->getEventRouter().signalEvent(me);
                        // update LED
                        if (instance->ledStatus) {
                            if (b == instance->buttonOn) {
                                lv_led_on(instance->ledStatus);
                            } else {
                                lv_led_off(instance->ledStatus);
                            }
                        }
                    }
                }
            }
        }
        static void ui_lock_event_handler(lv_event_t* e) {
            auto instance = static_cast<LevelControlActivity*>(lv_event_get_user_data(e));
            if (instance) {
                lv_event_code_t code = lv_event_get_code(e);
                if (code == LV_EVENT_RELEASED) {
                    instance->unlock();
                } else if (code == LV_EVENT_PRESSED) {
                    instance->lock();
                }
            }
        }

    };

}}}

#endif // DISABLE_LVGL

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_LEVELCONTROLACTIVITY_H
