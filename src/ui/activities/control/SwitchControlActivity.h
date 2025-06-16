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

#ifndef HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H
#define HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H

#ifdef ENABLE_UI

#include "service/api/APIRequest.h"
#include "service/api/devices/Dimmer.h"

#include "ui/Activity.h"
#include "ui/AnimationHelper.h"
#include "ui/Utility.h"
#include "ui/components/RoundButton.h"

namespace UI { namespace Activities { namespace Control {

    using namespace Service::API;
    using namespace Service::API::devices;
    using namespace Service::ModuleApi;
    using namespace Service::WidgetApi;
    using namespace UI::Components;

    enum SwitchControlButtons {
        CONTROL_BUTTON_ADD = 2,
        CONTROL_BUTTON_SUB = 0,
        CONTROL_BUTTON_ON = 3,
        CONTROL_BUTTON_OFF = 1,
        CONTROL_BUTTON_TOGGLE = 4
    };

    class SwitchControlActivity : public Activity, public APIHandler {
    public:
        LinkedList<Module*> moduleList;
        Module module;

        explicit SwitchControlActivity(const char* moduleAddress);

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
        void onDraw() override;

        void onPointerDown(PointerEvent e) override;
        void onPointerUp(PointerEvent e) override;
        void onPan(PanEvent e) override;

    private:
        LinkedList<Point> buttonOffBounds = LinkedList<Point>();
        LinkedList<Point> buttonOnBounds = LinkedList<Point>();
        LinkedList<Point> buttonIncBounds = LinkedList<Point>();
        LinkedList<Point> buttonDecBounds = LinkedList<Point>();
        RoundButton* toggleButton{};
        unsigned int selectedButton = -1;
        unsigned long showLoaderTs{};
        unsigned int currentLevel = 50; // 0-100
        bool isSwitchedOn = false;
        unsigned long errorReportTs{};
        int32_t diameter = 240;

        void drawNavigationButtons();

        void emitLevelEvent();
    };

}}}

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H
