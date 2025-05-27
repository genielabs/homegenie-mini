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

#include "Common.h"

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

        explicit SwitchControlActivity(const char* moduleAddress) {
            setDrawInterval(100); // Task.h

            // diameter 240

            // OFF button
            buttonOffBounds.add(Point{0, 0});
            buttonOffBounds.add(Point{0, 239});
            buttonOffBounds.add(Point{80, 150});
            buttonOffBounds.add(Point{80, 90});
            // ON button
            buttonOnBounds.add(Point{239, 0});
            buttonOnBounds.add(Point{239, 239});
            buttonOnBounds.add(Point{159, 150});
            buttonOnBounds.add(Point{159, 90});
            // INC button (+)
            buttonIncBounds.add(Point{0, 0});
            buttonIncBounds.add(Point{80, 90});
            buttonIncBounds.add(Point{159, 90});
            buttonIncBounds.add(Point{239, 0});
            // DEC button (-)
            buttonDecBounds.add(Point{80, 150});
            buttonDecBounds.add(Point{0, 239});
            buttonDecBounds.add(Point{239, 239});
            buttonDecBounds.add(Point{159, 150});

            module.domain  = IO::IOEventDomains::HomeAutomation_HomeGenie;
            module.address = moduleAddress;
            module.type    = ModuleType::Sensor;
            // load stored name for this module
            auto key = String(CONFIG_KEY_ACTIVITY_TITLE); key.concat(moduleAddress);
            module.name = Config::getSetting(key.c_str(), moduleAddress);
            module.setProperty(Implements::Scheduling, "true");
            module.setProperty(Implements::Scheduling_ModuleEvents, "true");
            module.onNameUpdated = [this](const char* oldName, const char* newName) {
                if (toggleButton != nullptr) {
                    toggleButton->setTitle(newName);
                }
                auto key = String(CONFIG_KEY_ACTIVITY_TITLE);
                key.concat(module.address);
                Config::saveSetting(key.c_str(), newName);
                refresh();
            };
            moduleList.add(&module);
            HomeGenie::getInstance()->addAPIHandler(this);
        }

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

        void attach(LGFX_Device* displayDevice) override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void onPointerDown(PointerEvent e) override {
            selectedButton = -1;
            auto ox = round((float)(canvas->width() - diameter) / 2.0f);
            auto oy = round((float)(canvas->height() - diameter) / 2.0f);
            auto p = Point{e.x - ox, e.y - oy};
            if (Utility::isInside(p, buttonOffBounds)) {
                selectedButton = CONTROL_BUTTON_OFF;
                refresh();
            } else if (Utility::isInside(p, buttonOnBounds)) {
                selectedButton = CONTROL_BUTTON_ON;
                refresh();
            } else if (Utility::isInside(p, buttonIncBounds)) {
                selectedButton = CONTROL_BUTTON_ADD;
                refresh();
            } else if (Utility::isInside(p, buttonDecBounds)) {
                selectedButton = CONTROL_BUTTON_SUB;
                refresh();
            } else if (toggleButton->hitTest(p.x, p.y)) {
                selectedButton = CONTROL_BUTTON_TOGGLE;
                refresh();
            }
        }
        void onPointerUp(PointerEvent e) override {
            if (selectedButton != -1) {
                showLoaderTs = millis();

                switch (selectedButton) {
                    case CONTROL_BUTTON_ADD: {
                        if (currentLevel == 100) break;
                        currentLevel += isSwitchedOn ? 10 : 0;
                        if (currentLevel > 100) currentLevel = 100;
                        signalLevel();
                        isSwitchedOn = true;
                    }
                    break;
                    case CONTROL_BUTTON_SUB: {
                        if (currentLevel == 10) break;
                        currentLevel -= isSwitchedOn ? 10 : 0;
                        if (currentLevel < 10) currentLevel = 10;
                        signalLevel();
                        isSwitchedOn = true;
                    }
                    break;
                    case CONTROL_BUTTON_ON: {
                        signalButton("on");
                        isSwitchedOn = true;
                    }
                    break;
                    case CONTROL_BUTTON_OFF: {
                        signalButton("off");
                        isSwitchedOn = false;
                    }
                    break;
                    case CONTROL_BUTTON_TOGGLE: {
                        signalButton("toggle");
                        isSwitchedOn = !isSwitchedOn;
                    }
                    break;
                }

                selectedButton = -1;
                refresh();
            }
        }
        void onPan(PanEvent e) override {
            if (selectedButton != -1) {
                selectedButton = -1;
                refresh();
            }
        }

    private:
        LinkedList<Point> buttonOffBounds = LinkedList<Point>();
        LinkedList<Point> buttonOnBounds = LinkedList<Point>();
        LinkedList<Point> buttonIncBounds = LinkedList<Point>();
        LinkedList<Point> buttonDecBounds = LinkedList<Point>();
        RoundButton* toggleButton = nullptr;
        unsigned int selectedButton = -1;
        unsigned long showLoaderTs = 0;
        unsigned int currentLevel = 50; // 0-100
        bool isSwitchedOn = false;
        unsigned long errorReportTs = 0;
        int32_t diameter = 240;

        void drawNavigationButtons() {
            canvas->setFont(&fonts::DejaVu24);
            auto hw = (int32_t)round((float)diameter / 2.0f);
            auto ox = (int32_t)round((float)(canvas->width() - diameter) / 2.0f);
            auto oy = (int32_t)round((float)(canvas->height() - diameter) / 2.0f);
            int toggleButtonRadius = (int)round((float)hw / 2.5f);
            for (int i = 0; i < 100; i += 25) {
                uint8_t buttonIndex = (i / 25);
                float angle = 45.0f + (i * 3.6f);
                canvas->fillArc(ox + hw, oy + hw, hw - 2,  toggleButtonRadius + 3, angle + 3, angle + 87, selectedButton == buttonIndex ? 1 : 0);
                canvas->fillArc(ox + hw, oy + hw, hw - 2,  toggleButtonRadius + 3, angle - 2, angle + 2, 1);
                canvas->setTextColor((selectedButton == buttonIndex) ? 0 : 1);
                switch (buttonIndex) {
                case CONTROL_BUTTON_SUB:
                    // DEC button label
                    canvas->drawCenterString("_", ox + hw, oy + 180);
                    break;
                case CONTROL_BUTTON_ADD:
                    // INC button label
                    canvas->drawCenterString("+", ox + hw, oy + 28);
                    break;
                case CONTROL_BUTTON_OFF:
                    // OFF button label
                    canvas->drawCenterString("OFF", ox + 38, oy + hw - 10);
                    break;
                case CONTROL_BUTTON_ON:
                    // ON button label
                    canvas->drawCenterString("ON", ox + 204, oy + hw - 10);
                    break;
                }
            }
        }

        void signalLevel() {
            auto l = (float)currentLevel / 100.0f;
            auto level = String(l, 3);
            module.setProperty(EVENT_SENSOR_LEVEL, level);
            auto m = QueuedMessage(&module, EVENT_SENSOR_LEVEL, level, &l, IO::IOEventDataType::Number);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }
        void signalButton(const char* button) {
            module.setProperty(EVENT_SENSOR_BUTTON, button);
            auto l = String(currentLevel);
            auto m = QueuedMessage(&module, EVENT_SENSOR_BUTTON, button, nullptr, IO::IOEventDataType::Text);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }

        void refresh() {
            drawNavigationButtons();
            invalidate();
        }
    };

}}}

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H
