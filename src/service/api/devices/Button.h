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

#ifndef HOMEGENIE_MINI_BUTTON_H
#define HOMEGENIE_MINI_BUTTON_H

#include <HomeGenie.h>

namespace Service { namespace API { namespace devices {

    enum ButtonStatus {
        BUTTON_STATUS_NOT_SET = -1,
        BUTTON_STATUS_NORMAL,
        BUTTON_STATUS_PRESSED,
    };
    enum ButtonGesture {
        BUTTON_GESTURE_CLICK,
        //BUTTON_GESTURE_DOUBLE_CLICK,
        BUTTON_GESTURE_LONG_PRESS
    };
    enum PullResistor {
        NONE = INPUT,
        PULL_UP = INPUT_PULLUP,
#ifndef ESP8266
        PULL_DOWN = INPUT_PULLDOWN
#endif
    };

    class Button: public Task, public APIHandler {
    public:
        Button(const char* domain, const char* address, const char* name, uint8_t pin, PullResistor mode = PULL_UP);
        void init() override;
        void loop() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest*, ResponseCallback*) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;
        void onSetStatus(std::function<void(ButtonStatus)> callback) {
            setStatusCallback = std::move(callback);
        }
        void onGesture(std::function<void(ButtonGesture)> callback) {
            onGestureCallback = std::move(callback);
        }
        bool isPressed() {
            return status == BUTTON_STATUS_PRESSED;
        }

        Module* module;
    private:
        LinkedList<Module*> moduleList;
        std::function<void(ButtonStatus)> setStatusCallback = nullptr;
        std::function<void(ButtonGesture)> onGestureCallback = nullptr;
        uint8_t pinNumber;
        unsigned long pressedTs = 0;
    protected:
        ButtonStatus status = BUTTON_STATUS_NOT_SET;
        float onLevel = 1;
    };

}}}

#endif //HOMEGENIE_MINI_BUTTON_H
