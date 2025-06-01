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

#ifndef HOMEGENIE_MINI_COLORCONTROLACTIVITY_H
#define HOMEGENIE_MINI_COLORCONTROLACTIVITY_H

#ifdef ENABLE_UI

#include "service/api/APIRequest.h"
#include "service/api/devices/Dimmer.h"

#include "ui/LvglActivity.h"
#include "ui/fonts/material_symbols_common.h"

#ifndef DISABLE_LVGL

#include "ui/LvglDriver.h"

namespace UI { namespace Activities { namespace Control {

    using namespace Service::API;
    using namespace Service::API::devices;
    using namespace Service::ModuleApi;
    using namespace Service::WidgetApi;

    class ColorControlData {
    public:
        int16_t hue = 0;
        int16_t saturation = 100;
        int16_t value = 100;

        bool isOn = false;

        uint8_t controlMode = 0;
    };

    class ColorControlActivity : public LvglActivity, public APIHandler {
    public:
        LinkedList<Module*> moduleList;
        Module module;

        explicit ColorControlActivity(const char* moduleAddress);

        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest* request, ResponseCallback* callback) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override;
        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;

        void onResume() override;
        void onPause() override;

    private:
        lv_style_t* backgroundGradient{};
        lv_obj_t* labelTitle{};
        lv_obj_t* labelLevel{};
        lv_obj_t* colorWheel{};
        lv_obj_t* circularSlider{};
        lv_obj_t* toggleButton{};
        lv_obj_t* ledStatus{};
        lv_obj_t* buttonColor{};
        lv_obj_t* buttonSaturation{};
        lv_obj_t* buttonValue{};

        ColorControlData data;
        int diameter{};
        int shiftY{};

        enum EditMode {
            EDIT_HUE,
            EDIT_SATURATION,
            EDIT_VALUE
        };

        void refreshData();
        void emitHsvEvent();
        void emitLevelEvent();
        void initRoundButton(lv_obj_t* button);

        static void activityEventsLock(lv_obj_t* obj, void* instance);
        static void uiLockEventHandler(lv_event_t* e);

        static void modeButtonEventHandler(lv_event_t* e);
        static void toggleButtonEventHandler(lv_event_t* e);
        static void colorWheelEventHandler(lv_event_t* e);
        static void circularSliderEventHandler(lv_event_t* e);

        static String hsvToNormalizedString(const lv_color_hsv_t& hsv_color);
        static void addButtonIcon(lv_obj_t* button, const char* symbol);
    };

}}}

#endif // DISABLE_LVGL

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_COLORCONTROLACTIVITY_H
