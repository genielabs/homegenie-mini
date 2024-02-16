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

#ifndef HOMEGENIE_MINI_EVENTS_H
#define HOMEGENIE_MINI_EVENTS_H

struct Event {
public:
    bool handled = false;
    void* target = nullptr;
};

struct PointerEvent: public Event {
public:
    PointerEvent(float x, float y, void* sender = nullptr) : Event() {
        this->x = x;
        this->y = y;
    };
    float x;
    float y;
};

class PointerHandler {
public:
    virtual void pointerDown(float x, float y) = 0;
    virtual void pointerMove(float x, float y) = 0;
    virtual void pointerUp(float x, float y) = 0;
};

class PointerListener {
public:
    virtual void onPointerDown(PointerEvent e) = 0;
    virtual void onPointerUp(PointerEvent e) = 0;
    virtual void onPointerMove(PointerEvent e) = 0;
};

#endif //HOMEGENIE_MINI_EVENTS_H
