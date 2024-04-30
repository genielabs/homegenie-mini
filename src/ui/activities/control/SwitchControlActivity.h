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

#ifndef HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H
#define HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H

#include "defs.h"

#ifndef DISABLE_UI

#include <HTTPClient.h>

#include "service/api/APIRequest.h"
#include "ui/Activity.h"
#include "ui/components/RoundButton.h"
#include "ui/AnimationHelper.h"
#include "ui/Utility.h"

namespace UI { namespace Activities { namespace Control {

    using namespace Service::API;
    using namespace UI::Components;

    enum SwitchControlButtons {
        CONTROL_BUTTON_ADD = 2,
        CONTROL_BUTTON_SUB = 0,
        CONTROL_BUTTON_ON = 3,
        CONTROL_BUTTON_OFF = 1
    };

    class SwitchControlActivity : public Activity {
    public:
        SwitchControlActivity() {
            setDrawInterval(100); // Task.h

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

        }
        void attach(lgfx::LGFX_Device* displayDevice) override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void onPointerDown(PointerEvent e) override {
            auto p = Point{e.x, e.y};
            if (Utility::isInside(p, buttonOffBounds)) {
                selectedButton = CONTROL_BUTTON_OFF;
                refresh();
            }
            if (Utility::isInside(p, buttonOnBounds)) {
                selectedButton = CONTROL_BUTTON_ON;
                refresh();
            }
            if (Utility::isInside(p, buttonIncBounds)) {
                selectedButton = CONTROL_BUTTON_ADD;
                refresh();
            }
            if (Utility::isInside(p, buttonDecBounds)) {
                selectedButton = CONTROL_BUTTON_SUB;
                refresh();
            }
        }
        void onPointerUp(PointerEvent e) override {
            if (selectedButton != -1) {
// TODO: fire event onNavigationButtonClicked(selectedButton) ...

                showLoaderTs = millis();

                switch (selectedButton) {
                    case 2: {
                        if (currentLevel == 100) break;
                        currentLevel += isSwitchedOn ? 10 : 0;
                        if (currentLevel > 100) currentLevel = 100;
                        bool success = sendCommand(ControlApi::Control_Level, String(currentLevel).c_str());
                        if (success) isSwitchedOn = true;
                    }
                        break;
                    case 0: {
                        if (currentLevel == 10) break;
                        currentLevel -= isSwitchedOn ? 10 : 0;
                        if (currentLevel < 10) currentLevel = 10;
                        bool success = sendCommand(ControlApi::Control_Level, String(currentLevel).c_str());
                        if (success) isSwitchedOn = true;
                    }
                        break;
                    case 3: {
                        bool success = sendCommand(ControlApi::Control_On);
                        if (success) isSwitchedOn = true;
                    }
                        break;
                    case 1: {
                        bool success = sendCommand(ControlApi::Control_Off);
                        if (success) isSwitchedOn = false; // off
                        break;
                    }
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

        void setModuleUrl(String baseUrl) {
            this->moduleBaseUrl = baseUrl;
        }

    private:
        HTTPClient http;
        LinkedList<Point> buttonOffBounds = LinkedList<Point>();
        LinkedList<Point> buttonOnBounds = LinkedList<Point>();
        LinkedList<Point> buttonIncBounds = LinkedList<Point>();
        LinkedList<Point> buttonDecBounds = LinkedList<Point>();
        RoundButton* toggleButton = nullptr;
        unsigned int selectedButton = -1;
        unsigned long showLoaderTs = 0;
        unsigned int currentLevel = 50; // 0-100
        bool isSwitchedOn = false;
        String moduleBaseUrl;
        unsigned long errorReportTs = 0;

        void drawNavigationButtons() {
            canvas->setFont(&fonts::DejaVu24);
            int hw = (int)round((float)canvas->width() / 2.0f);
            int toggleButtonRadius = (int)round((float)hw / 2.5f);
            for (int i = 0; i < 100; i += 25) {
                uint8_t buttonIndex = (i / 25);
                float angle = 45.0f + (i * 3.6f);
                canvas->fillArc(hw, hw, hw - 2,  toggleButtonRadius + 3, angle + 3, angle + 87, selectedButton == buttonIndex ? 1 : 0);
                canvas->fillArc(hw, hw, hw - 2,  toggleButtonRadius + 3, angle - 2, angle + 2, 1);
                canvas->setTextColor((selectedButton == buttonIndex) ? 0 : 1);
                switch (buttonIndex) {
                case CONTROL_BUTTON_SUB:
                    // DEC button label
                    canvas->drawCenterString("_", hw, 180);
                    break;
                case CONTROL_BUTTON_ADD:
                    // INC button label
                    canvas->drawCenterString("+", hw, 28);
                    break;
                case CONTROL_BUTTON_OFF:
                    // OFF button label
                    canvas->drawCenterString("OFF", 38, hw - 10);
                    break;
                case CONTROL_BUTTON_ON:
                    // ON button label
                    canvas->drawCenterString("ON", 204, hw - 10);
                    break;
                }
            }
        }

        void refresh() {
            drawNavigationButtons();
            invalidate();
        }

        // TODO: implement with Automation::Helpers::NetHelper::httpGet
        bool sendCommand(const char* command, const char* options = "") {
            String url = moduleBaseUrl;
            if (!url.endsWith("/")) {
                url += String("/");
            }
            url += String(command) + "/" + String(options);
            http.begin(url.c_str());
            //http.addHeader("Content-Type", "application/x-www-form-urlencoded");
            int httpCode = http.GET();
            if (httpCode > 0) {
                // Server response
                if (httpCode == HTTP_CODE_OK) {
                    String response = http.getString();
                    // TODO: parse response
                    //Serial.println(response);
                    return true;
                } else {
                    // Server reported error code
                    //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
                }
            } else {
                //Serial.printf("[HTTP] GET... failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
            }
            errorReportTs = millis();
            return false;
        }

    };

}}}

#endif // DISABLE_UI

#endif //HOMEGENIE_MINI_SWITCHCONTROLACTIVITY_H
