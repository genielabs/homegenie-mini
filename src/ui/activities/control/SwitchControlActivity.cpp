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

#include "SwitchControlActivity.h"

#ifdef ENABLE_UI

namespace UI { namespace Activities { namespace Control {

    SwitchControlActivity::SwitchControlActivity(const char *moduleAddress) {
        setDrawInterval(100); // Task.h
        setColorDepth(lgfx::palette_2bit);

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

    void SwitchControlActivity::init() {
    }

    bool SwitchControlActivity::canHandleDomain(String *domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool SwitchControlActivity::handleRequest(APIRequest *request, ResponseCallback *callback) {
        return false;
    }

    bool SwitchControlActivity::handleEvent(IIOEventSender *, const char *domain, const char *address,
                                            const char *eventPath, void *eventData, IOEventDataType) {
        return false;
    }

    Module *SwitchControlActivity::getModule(const char *domain, const char *address) {
        if (module.domain.equals(domain) && module.address.equals(address))
            return &module;
        return nullptr;
    }

    LinkedList<Module *> *SwitchControlActivity::getModuleList() {
        return &moduleList;
    }

    void SwitchControlActivity::onPointerDown(PointerEvent e) {
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

    void SwitchControlActivity::onPointerUp(PointerEvent e) {
        if (selectedButton != -1) {
            showLoaderTs = millis();

            switch (selectedButton) {
                case CONTROL_BUTTON_ADD: {
                    if (currentLevel == 100) break;
                    currentLevel += isSwitchedOn ? 10 : 0;
                    if (currentLevel > 100) currentLevel = 100;
                    isSwitchedOn = true;
                    emitLevelEvent();
                } break;
                case CONTROL_BUTTON_SUB: {
                    if (currentLevel == 10) break;
                    currentLevel -= isSwitchedOn ? 10 : 0;
                    if (currentLevel < 10) currentLevel = 10;
                    isSwitchedOn = true;
                    emitLevelEvent();
                } break;
                case CONTROL_BUTTON_ON: {
                    isSwitchedOn = true;
                    emitLevelEvent();
                } break;
                case CONTROL_BUTTON_OFF: {
                    isSwitchedOn = false;
                    emitLevelEvent();
                } break;
                case CONTROL_BUTTON_TOGGLE: {
                    isSwitchedOn = !isSwitchedOn;
                    emitLevelEvent();
                } break;
            }

            selectedButton = -1;
            refresh();
        }
    }

    void SwitchControlActivity::onPan(PanEvent e) {
        if (selectedButton != -1) {
            selectedButton = -1;
            refresh();
        }
    }

    void SwitchControlActivity::onResume() {

        // text color
        canvas->setPaletteColor(1, LGFX_Sprite::color565(255, 255, 255));
        // accent color
        canvas->setPaletteColor(2, LGFX_Sprite::color565(40, 200, 40));
        // warn color
        canvas->setPaletteColor(3, LGFX_Sprite::color565(255, 20, 20));

        auto hw = (int32_t)round((float)diameter / 2.0f);
        auto ox = (int32_t)round((float)(canvas->width() - diameter) / 2.0f);
        auto oy = (int32_t)round((float)(canvas->height() - diameter) / 2.0f);

        canvas->drawCircle(ox + hw, oy + hw, hw, 1);
        canvas->drawCircle(ox + hw, oy + hw, hw - 1, 1);

        if (toggleButton == nullptr) {
            // add input controls to this activity
            auto toggleButtonRadius = (uint16_t)round((float)hw / 1.25f); // 96px on a 240w display
            toggleButton = new RoundButton(canvas, ox + hw, oy + hw, toggleButtonRadius);
            toggleButton->setTitle(module.name.c_str());
            addControl((InputControl*)toggleButton);
        } else {
            toggleButton->setLocation(ox + hw, oy + hw);
        }

        drawNavigationButtons();

    }

    void SwitchControlActivity::onPause() {

    }

    void SwitchControlActivity::onDraw() {
        if (toggleButton != nullptr) {

            if (showLoaderTs > 0) {
                setDrawInterval(0); // RT PRIORITY - loop
                float duration = 350.0f;
                float elapsed = millis() - showLoaderTs;
                if (elapsed <= duration) {
                    float p = AnimationHelper::animateFloat(0, 100, elapsed / duration, ANIMATION_EASING_LINEAR);
                    toggleButton->setProgress(p);
                } else {
                    showLoaderTs = 0;
                    toggleButton->setProgress(-1);
                    setDrawInterval(100); // 100ms loop interval
                }
            }
            // button color (red=3 for signaling error, green=2 for success)
            if (millis() - errorReportTs <= 500) {
                toggleButton->setColor(3);
            } else {
                toggleButton->setColor(2);
            }
            // button labels
            String level = "OFF";
            if (isSwitchedOn) {
                level = String(currentLevel) + "%";
            }
            toggleButton->setSubtitle(level.c_str());

        }
    }

    void SwitchControlActivity::drawNavigationButtons() {
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

    void SwitchControlActivity::emitLevelEvent() {
        // Emit event
        auto l = (float)currentLevel / 100.0f;
        auto level = isSwitchedOn ? String(l, 3) : String("0");
        module.setProperty(IOEventPaths::Sensor_Level, level);
        auto me = QueuedMessage(&module, IOEventPaths::Sensor_Level, level);
        HomeGenie::getInstance()->getEventRouter().signalEvent(me);
    }

    void SwitchControlActivity::refresh() {
        drawNavigationButtons();
        invalidate();
    }

}}}

#endif // ENABLE_UI
