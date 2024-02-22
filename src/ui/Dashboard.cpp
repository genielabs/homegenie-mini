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

#include "Dashboard.h"

#ifndef DISABLE_UI

#include "GestureHelper.h"
#include "AnimationHelper.h"

void Dashboard::loop() {
    auto activity = getForegroundActivity();

    // flying
    if (!pointerDown && lastTouchPoint.shiftX != 0 && nextActivity != nullptr) {
        unsigned long startTime = millis();
        unsigned long duration = 150;

        bool switchActivity = (swipeDirection != TOUCH_DIRECTION_NONE);
        float currentShiftX = lastTouchPoint.shiftX;
        int direction = (lastTouchPoint.direction == TOUCH_DIRECTION_LEFT ? -1 : 1);

        while (millis() - startTime < duration) {
            float progress = (float)(millis() - startTime) / (float)duration;
            float cx;
            if (switchActivity) {
                float scrollTo = display->width() * direction;
                cx = AnimationHelper::animateFloat(currentShiftX, scrollTo, progress, ANIMATION_EASING_EASE_IN_OUT);
            } else {
                cx = AnimationHelper::animateFloat(currentShiftX, 0, progress, ANIMATION_EASING_LINEAR);
            }
            if (nextActivity != nullptr) {
                nextActivity->setDrawOffset(cx - (float)(display->width() * direction * (switchActivity ? 1 : -1)), 0);
            }
            if (activity != nullptr) {
                activity->setDrawOffset(cx, 0);
            }
        }

        if (switchActivity) {
            setForegroundActivity(nextActivity);
        } else {
            if (nextActivity != nullptr) {
                nextActivity->pause();
            }
            if (activity != nullptr) {
                activity->setDrawOffset(0, 0);
            }
        }
        nextActivity = nullptr;
    }


    // check touch input
    lgfx::touch_point_t tp;
    display->getTouch(&tp);
    if (tp.size > 0) {
        if (!pointerDown) {
            pointerDown = true;
            swipeDirection = TOUCH_DIRECTION_NONE;
            gestureHelper->pointerDown(tp.x, tp.y);
            if (activity != nullptr) {
                activity->pointerDown(tp.x, tp.y);
            }
        } else if (tp.x != lastTp.x || tp.y != lastTp.y) {
            lastTp.x = tp.x;
            lastTp.y = tp.y;
            gestureHelper->pointerMove(tp.x, tp.y);
            if (activity != nullptr) {
                activity->pointerMove(tp.x, tp.y);
            }
        }
        Service::PowerManager::setActive();
    } else if (pointerDown) {
        pointerDown = false;
        gestureHelper->pointerUp(tp.x, tp.y);
        if (activity != nullptr) {
            activity->pointerUp(tp.x, tp.y);
        }
    }
}

void Dashboard::addActivity(Activity* activity) {
    activity->attach(display);
    //activity->pause();
    bool exists = false;
    for (const auto &item: activityList) {
        if (item == activity) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        activityList.add(activity);
    }
}
void Dashboard::setForegroundActivity(Activity* activity) {
    int idx = 0;
    for (const auto &a: activityList) {
        if (a == activity) {
            currentActivityIndex = idx;
        }
        a->pause();
        idx++;
    }
    activity->resume();
    activity->setDrawOffset(0, 0);
}
Activity* Dashboard::getForegroundActivity() {
    return currentActivityIndex != -1 ? activityList.get(currentActivityIndex) : nullptr;
}

void Dashboard::onTap(PointerEvent e) {
    auto activity = getForegroundActivity();
    if (activity != nullptr) {
        activity->tap(e);
    }
    /*
    if (!e.handled) {
        if (currentActivityIndex > -1 && currentActivityIndex < activityList.size() - 1) {
            auto nextActivity = activityList.get(currentActivityIndex + 1);
            setForegroundActivity(nextActivity);
        } else if (activityList.size() > 0) {
            currentActivityIndex = 0;
            setForegroundActivity(activityList.get(currentActivityIndex));
        }
    }
    */
}
void Dashboard::onSwipe(SwipeEvent e) {
    if (e.direction == TOUCH_DIRECTION_LEFT || e.direction == TOUCH_DIRECTION_RIGHT) {
        swipeDirection = e.direction;
    }
}
void Dashboard::onPan(PanEvent e) {
    auto activity = getForegroundActivity();
    if (activity != nullptr) {

        activity->pan(e);

        if (activityList.size() > 1 && (e.touchPoint.direction == TOUCH_DIRECTION_LEFT || e.touchPoint.direction == TOUCH_DIRECTION_RIGHT)) {
            auto next = (e.touchPoint.shiftX > 0) ? getNextActivity() : getPreviousActivity();
            if (next != nextActivity && nextActivity != nullptr) {
                nextActivity->pause();
            }
            nextActivity = next;
            nextActivity->resume();
            nextActivity->setDrawOffset(e.touchPoint.shiftX + (float)display->width()*(e.touchPoint.shiftX > 0 ? -1 : 1), 0);

            activity->setDrawOffset(e.touchPoint.shiftX, 0);
        }

        lastTouchPoint = e.touchPoint;
    }
}
void Dashboard::onTouch(PointerEvent e) {

}
void Dashboard::onRelease(PointerEvent e) {

}
Activity* Dashboard::getNextActivity() {
    int nextActivityIndex = 0;
    if (currentActivityIndex + 1 < activityList.size()) {
        nextActivityIndex = currentActivityIndex + 1;
    }
    return activityList.get(nextActivityIndex);
}
Activity* Dashboard::getPreviousActivity() {
    unsigned int prevActivityIndex;
    if (currentActivityIndex - 1 >= 0) {
        prevActivityIndex = currentActivityIndex - 1;
    } else {
        prevActivityIndex = activityList.size() - 1;
    }
    return activityList.get(prevActivityIndex);
}

#endif // DISABLE_UI
