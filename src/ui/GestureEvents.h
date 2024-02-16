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

#ifndef HOMEGENIE_MINI_GESTUREEVENTS_H
#define HOMEGENIE_MINI_GESTUREEVENTS_H

#include "Events.h"

enum ScrollMode {
    SCROLL_MODE_NONE = 0,
    SCROLL_MODE_HORIZONTAL = 1,
    SCROLL_MODE_VERTICAL = 2
};

enum Gesture {
    GESTURE_NONE,
    GESTURE_TAP,
    GESTURE_SWIPE
};
enum TouchDirection {
    TOUCH_DIRECTION_NONE,
    TOUCH_DIRECTION_CANCEL,
    TOUCH_DIRECTION_LEFT,
    TOUCH_DIRECTION_RIGHT,
    TOUCH_DIRECTION_UP,
    TOUCH_DIRECTION_DOWN
};

struct SpeedData {
    unsigned long time;
    float x;
    float y;
};

struct TouchPoint {
public:
    float x = 0;
    float y = 0;
    // initial touch position
    float startX;
    float startY;
    unsigned long startTime;
    // relative movement
    float shiftX;
    float shiftY;
    unsigned long endTime;
    // relative movement at every step
    float stepX;
    float stepY;
    float stepDistance;
    float stepSpeed;
    unsigned long stepTime;
    // actual position and speed
    double velocity;
    TouchDirection direction;
    float distance;
};

struct SwipeEvent: public Event {
public:
    SwipeEvent(TouchDirection d) {
        direction = d;
    }
    TouchDirection direction;
};
struct PanEvent: public Event {
public:
    PanEvent(TouchPoint tp) {
        touchPoint = tp;
    }
    TouchPoint touchPoint;
};

class GestureListener {
public:
    virtual void onTap(PointerEvent e) = 0;
    virtual void onSwipe(SwipeEvent e) = 0;
    virtual void onPan(PanEvent e) = 0;
    virtual void onTouch(PointerEvent e) = 0;
    virtual void onRelease(PointerEvent e) = 0;
};

#endif //HOMEGENIE_MINI_GESTUREEVENTS_H
