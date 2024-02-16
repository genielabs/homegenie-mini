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

#ifndef HOMEGENIE_MINI_INPUTCONTROL_H
#define HOMEGENIE_MINI_INPUTCONTROL_H

#include "Config.h"

#include "GestureEvents.h"

namespace UI {

    class InputControl: public PointerListener, public GestureListener {
    public:
        virtual void draw() = 0;
        virtual bool hitTest(float x, float y) = 0;
        void onPointerDown(PointerEvent e) override {}
        void onPointerUp(PointerEvent e) override {}
        void onPointerMove(PointerEvent e) override {}
        void onTap(PointerEvent e) override {}
        void onTouch(PointerEvent e) override {};
        void onPan(PanEvent e) override {};
        void onRelease(PointerEvent e) override {};
        bool isPressed = false;
    private:
        void onSwipe(SwipeEvent e) override {};
    };

}

#endif //HOMEGENIE_MINI_INPUTCONTROL_H
