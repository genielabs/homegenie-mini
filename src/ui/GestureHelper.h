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
 * Note:
 * This code is an adaption from one of my other open-source
 * works: zuix.js `gesture-helper` class.
 * https://github.com/zuixjs/zkit/blob/master/source/lib/1.2/controllers/gesture-helper.js
 *
 */

#ifndef HOMEGENIE_MINI_GESTUREHELPER_H
#define HOMEGENIE_MINI_GESTUREHELPER_H

#include <cmath>

#include "Config.h"
#include "GestureEvents.h"

class GestureHelper: public PointerHandler {

public:
    GestureHelper(GestureListener* l) {
        listener = l;
    }

    void pointerDown(float x, float y) override {
        if (x > startGap) {
            isPointerDown = true;
            ignoreSession = false;
            // targetElement.css('touch-action', 'none');
            TouchStart(x, y);
        } else ignoreSession = true;
    }
    void pointerMove(float x, float y) override {
        if (!ignoreSession && isPointerDown) {
            TouchMove(x, y);
        }
    }
    void pointerUp(float x, float y) override {
        if (!ignoreSession) {
            isPointerDown = false;
            TouchStop(x, y);
        }
    }

    unsigned long elapsed() const {
        return millis() - touchPointer.startTime;
    }
    Gesture gesture() {
        return currentGesture;
    }

private:
    static const unsigned int GESTURES_GUESS_SENSITIVITY = 4; // between 2-4 for best perf.
    static const unsigned int GESTURE_TAP_TIMEOUT = 750;
    // state vars
    ScrollMode scrollMode = SCROLL_MODE_NONE;
    TouchPoint touchPointer;
    bool ignoreSession = false;
    float startGap = -1;
    Gesture currentGesture = GESTURE_NONE;
    TouchDirection swipeDirection = TOUCH_DIRECTION_NONE;
    bool isPointerDown = false;
    unsigned long lastTapTime = millis();
    // speed meter data
    TouchDirection touchDirection;
    SpeedData startData;
    SpeedData stopData;
    // gesture events listener
    GestureListener* listener{};

    void TouchStart(float x, float y) {
        const unsigned long timestamp = millis();
        touchPointer.startX = x;
        touchPointer.startY = y;
        touchPointer.startTime = timestamp;
        touchPointer.endTime = timestamp;
        touchPointer.shiftX = 0;
        touchPointer.shiftY = 0;
        touchPointer.stepX = 0;
        touchPointer.stepY = 0;
        touchPointer.velocity = 0;
        touchPointer.stepTime = timestamp;
        touchPointer.velocity = 0;
        touchPointer.direction = TOUCH_DIRECTION_NONE;
        touchPointer.x = x;
        touchPointer.y = y;
        currentGesture = GESTURE_NONE;
        /// speed-meter
        touchDirection = TOUCH_DIRECTION_NONE;
        startData.time = 0;
        stopData.time = 0;
        listener->onTouch(PointerEvent(touchPointer.x, touchPointer.y));
    }
    void TouchMove(float x, float y) {
        touchPointer.x = x;
        touchPointer.y = y;
        touchPointer.shiftX = (x - touchPointer.startX);
        touchPointer.shiftY = (y - touchPointer.startY);
        touchPointer.endTime = millis();
        // detect actual gesture
        auto gesture = detectGesture();
        if (gesture != GESTURE_NONE /*&& currentGesture != GESTURE_NONE*/) {
            if (swipeDirection != TOUCH_DIRECTION_NONE && swipeDirection != touchPointer.direction) {
                // stop gesture detection if not coherent
                currentGesture = GESTURE_NONE;
                swipeDirection = TOUCH_DIRECTION_CANCEL;
            } else {
                currentGesture = gesture;
                swipeDirection = touchPointer.direction;
            }
        }
        if (touchDirection != TOUCH_DIRECTION_NONE) {
            listener->onPan(PanEvent(touchPointer));
        }
    }
    void TouchStop(float x, float y) {
        speedMeterUpdate();
        if (currentGesture == GESTURE_NONE) {
            currentGesture = detectGesture();
        }
        if (currentGesture != GESTURE_NONE) {
            switch (currentGesture) {
                case GESTURE_TAP:
                    listener->onTap(PointerEvent(touchPointer.x, touchPointer.y));
                    break;
                case GESTURE_SWIPE:
                    listener->onSwipe(SwipeEvent(touchPointer.direction));
                    break;
            }
        }
        listener->onRelease(PointerEvent(touchPointer.x, touchPointer.y));
        scrollMode = SCROLL_MODE_NONE;
        swipeDirection = TOUCH_DIRECTION_NONE;
    }

    Gesture detectGesture() {
        Gesture gesture = GESTURE_NONE;
        // update touchPointer.velocity data
        speedMeterUpdate();
        // update tap gesture and swipe direction
        const unsigned int minStepDistance = GESTURES_GUESS_SENSITIVITY; // <--- !!! this should not be greater than 2 for best performance
        const float angle = atan2(touchPointer.shiftY-touchPointer.stepY, touchPointer.shiftX-touchPointer.stepX) * 180.0f / PI;
        if ((touchPointer.shiftX) == 0 && (touchPointer.shiftY) == 0 && touchPointer.startTime > lastTapTime + 100 && touchPointer.stepTime < GESTURE_TAP_TIMEOUT) {
            // gesture TAP
            lastTapTime = millis();
            gesture = GESTURE_TAP;
        } else if ((scrollMode == SCROLL_MODE_NONE || scrollMode == SCROLL_MODE_HORIZONTAL) &&
                   touchPointer.stepDistance > minStepDistance && ((angle >= 135 && angle <= 180) || (angle >= -180 && angle <= -135))) {
            // gesture swipe RIGHT
            touchPointer.direction = TOUCH_DIRECTION_LEFT;
            gesture = GESTURE_SWIPE;
            scrollMode = SCROLL_MODE_HORIZONTAL;
        } else if ((scrollMode == SCROLL_MODE_NONE || scrollMode == SCROLL_MODE_HORIZONTAL) &&
                   touchPointer.stepDistance > minStepDistance && ((angle >= 0 && angle <= 45) || (angle >= -45 && angle < 0))) {
            // gesture swipe LEFT
            touchPointer.direction = TOUCH_DIRECTION_RIGHT;
            gesture = GESTURE_SWIPE;
            scrollMode = SCROLL_MODE_HORIZONTAL;
        } else if ((scrollMode == SCROLL_MODE_NONE || scrollMode == SCROLL_MODE_VERTICAL) &&
                   touchPointer.stepDistance > minStepDistance && (angle > 45 && angle < 135)) {
            // gesture swipe UP
            touchPointer.direction = TOUCH_DIRECTION_DOWN;
            gesture = GESTURE_SWIPE;
            scrollMode = SCROLL_MODE_VERTICAL;
        } else if ((scrollMode == SCROLL_MODE_NONE || scrollMode == SCROLL_MODE_VERTICAL) &&
                   touchPointer.stepDistance > minStepDistance && (angle > -135 && angle < -45)) {
            // gesture swipe DOWN
            touchPointer.direction = TOUCH_DIRECTION_UP;
            gesture = GESTURE_SWIPE;
            scrollMode = SCROLL_MODE_VERTICAL;
        }
        // reset touch step data
        if (touchPointer.stepDistance > minStepDistance) {
            speedMeterStep();
        }
        return gesture;
    }

    void speedMeterUpdate() {
        stopData.time = millis();
        stopData.x = touchPointer.x;
        stopData.y = touchPointer.y;
        if (touchDirection != touchPointer.direction) {
            speedMeterReset();
            return;
        } else if (touchDirection == TOUCH_DIRECTION_NONE) {
            touchDirection = touchPointer.direction;
        }
        unsigned long elapsedTime = stopData.time - startData.time;
        float lx = (stopData.x - startData.x);
        float ly = (stopData.y - startData.y);
        float d = sqrt(lx*lx + ly*ly);
        touchPointer.distance = d;
        // movement speed in px/ms
        float speed = (d / (float)elapsedTime);
        // add the direction info
        touchPointer.velocity = (touchPointer.direction == TOUCH_DIRECTION_LEFT || touchPointer.direction == TOUCH_DIRECTION_UP) ? -speed : speed;
        // update "step" speed data
        touchPointer.stepTime = (touchPointer.endTime - touchPointer.stepTime);
        lx = (touchPointer.shiftX - touchPointer.stepX);
        ly = (touchPointer.shiftY - touchPointer.stepY);
        touchPointer.stepDistance = sqrt(lx*lx+ly*ly);
        touchPointer.stepSpeed = (touchPointer.stepDistance / (float)touchPointer.stepTime);
    }
    void speedMeterStep() {
        touchPointer.stepTime = touchPointer.endTime;
        touchPointer.stepX = touchPointer.shiftX;
        touchPointer.stepY = touchPointer.shiftY;
        touchPointer.stepSpeed = 0;
        touchPointer.stepDistance = 0;
    }
    void speedMeterReset() {
        // direction changed: reset
        touchDirection = touchPointer.direction;
        startData.time = millis();
        startData.x = touchPointer.x;
        startData.y = touchPointer.y;
        touchPointer.velocity = 0;
        touchPointer.distance = 0;
        speedMeterStep();
    }

};

#endif //HOMEGENIE_MINI_GESTUREHELPER_H
