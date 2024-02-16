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

#ifndef HOMEGENIE_MINI_TOGGLEBUTTON_H
#define HOMEGENIE_MINI_TOGGLEBUTTON_H

#include <LinkedList.h>
#include <LovyanGFX.hpp>

#include "ui/InputControl.h"
#include "ui/Utility.h"

namespace UI { namespace Components {

    class PolyButton: InputControl {

    private:
        LGFX_Sprite* canvas;
        int cx, cy, w;
        LinkedList<Point>* bounds;

    public:
        PolyButton(LGFX_Sprite* c, LinkedList<Point>* b) {
            canvas = c;
            bounds = b;
        }

        bool hitTest(float x, float y) override {
            return Utility::isInside(Point{x, y}, *bounds);
        }
        void draw() override {

        }
    };

}}

#endif //HOMEGENIE_MINI_TOGGLEBUTTON_H
