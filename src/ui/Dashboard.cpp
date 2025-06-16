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

#ifdef ENABLE_UI

Dashboard::Dashboard(DisplayDriver *displayDriver, lgfx::color_depth_t colorDepth) {
    //setLoopInterval(1); // Task.h

    driver = displayDriver;
    display = driver->getDisplay();
    display->setColorDepth(colorDepth);
    display->initDMA();
    gestureHelper = new GestureHelper(this);

    // apply display preferences
    Preferences preferences;
    preferences.begin(CONFIG_SYSTEM_NAME, true);
    uint32_t displayRotation = preferences.getUInt(CONFIG_KEY_screen_rotation);
    preferences.end();
    setRotation(displayRotation);

    display->setBrightness(displayBrightness);
    display->drawCenterString("Hello World! =)", display->width() / 2, display->height() / 2);
#ifndef DISABLE_LVGL
    LvglDriver::begin(display);
#endif
}

void Dashboard::loop() {
    auto activity = getForegroundActivity();
    lgfx::touch_point_t tp;
    display->getTouch(&tp);

#ifdef ENABLE_SCREEN_SAVER
    // screen saver
    if (tp.size > 0) {
        if (isScreenSaverActive) {
            disableScreenSaver();
            return;
        } else if (isScreenDimmed) {
            isScreenDimmed = false;
            display->setBrightness(displayBrightness);
        }
    }
    if (screenSaverTimeoutMs > 0 && (!activity || !activity->isAlwaysOn())) {
        if ((millis() - lastTouchTs) > screenSaverTimeoutMs) {
            enableScreenSaver();
        } else if ((millis() - lastTouchTs) > screenSaverTimeoutMs / 2) {
            // dim screen
            isScreenDimmed = true;
            display->setBrightness(displayBrightness / 4);
        }
    }
#endif

    if (nextActivity) {
        nextActivity->refresh();
    }
    // flying
    if (!pointerDown && lastTouchPoint.shiftX != 0 && nextActivity) {
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
            nextActivity->setDrawOffset(cx - (float) (display->width() * direction * (switchActivity ? 1 : -1)), 0);
            if (activity) {
                activity->setDrawOffset(cx, 0);
            }
        }

        if (switchActivity) {
            setForegroundActivity(nextActivity);
        } else {
            nextActivity->pause();
            if (activity) {
                activity->setDrawOffset(0, 0);
#ifndef DISABLE_LVGL
                if (activity->isLvgl()) {
                    activity->pause();
                    activity->resume();
                }
#endif
            }
        }
        nextActivity = nullptr;
    }


    // check touch input
    if (tp.size > 0) {
#ifdef ENABLE_SCREEN_SAVER
        lastTouchTs = millis();
#endif
#ifdef CONFIG_ENABLE_POWER_MANAGER
        Service::PowerManager::setActive();
#endif
        if (!pointerDown) {
            pointerDown = true;
            swipeDirection = TOUCH_DIRECTION_NONE;
            gestureHelper->pointerDown(tp.x, tp.y);
            if (activity) {
                activity->pointerDown(tp.x, tp.y);
            }
        } else if (tp.x != lastTp.x || tp.y != lastTp.y) {
            lastTp.x = tp.x;
            lastTp.y = tp.y;
            gestureHelper->pointerMove(tp.x, tp.y);
            if (activity) {
                activity->pointerMove(tp.x, tp.y);
            }
        }
    } else if (pointerDown) {
        pointerDown = false;
        gestureHelper->pointerUp(tp.x, tp.y);
        if (activity) {
            activity->pointerUp(tp.x, tp.y);
        }
    }

    if (activity) {
        activity->refresh();
    }
}

void Dashboard::addActivity(Activity* activity) {
    activity->attach(driver);
    bool exists = false;
    for (const auto &item: activityList) {
        if (item == activity) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        activityList.add(activity);
        // set first activity as foreground
        if (activityList.size() == 1) {
            setForegroundActivity(activity);
        }
    }
}
void Dashboard::setForegroundActivity(Activity* activity) {
    int idx = 0;
    for (const auto &a: activityList) {
        if (a == activity) {
            currentActivityIndex = idx;
        } else {
            a->pause();
        }
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
    if (activity) {
        activity->tap(e);
    }
}
void Dashboard::onSwipe(SwipeEvent e) {
    if (e.direction == TOUCH_DIRECTION_LEFT || e.direction == TOUCH_DIRECTION_RIGHT) {
        swipeDirection = e.direction;
    }
}
void Dashboard::onPan(PanEvent e) {
    auto activity = getForegroundActivity();
    if (activity && !activity->isLocked()) {
#ifndef DISABLE_LVGL
        LvglDriver::disableInput = true;
#endif
        activity->pan(e);

        if (activityList.size() > 1 && (e.touchPoint.direction == TOUCH_DIRECTION_LEFT || e.touchPoint.direction == TOUCH_DIRECTION_RIGHT)) {
            auto next = (e.touchPoint.shiftX > 0) ?  getPreviousActivity() : getNextActivity();
            if (next != nextActivity && nextActivity) {
                nextActivity->pause();
            }
            nextActivity = next;
            nextActivity->resume();
            nextActivity->setDrawOffset(
                    e.touchPoint.shiftX + (float) display->width() * (e.touchPoint.shiftX > 0 ? -1 : 1), 0);
            activity->setDrawOffset(e.touchPoint.shiftX, 0);
        }

        lastTouchPoint = e.touchPoint;
    }
}
void Dashboard::onTouch(PointerEvent e) {
}
void Dashboard::onRelease(PointerEvent e) {
#ifndef DISABLE_LVGL
    if (LvglDriver::disableInput) {
        LvglDriver::disableInput = false;
    }
#endif
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

void Dashboard::invalidate() {
    for (const auto &item: activityList) {
        item->pause();
    }
    auto current = getForegroundActivity();
    if (current) {
        current->resume();
    }
}

LGFX_Device *Dashboard::getDisplay() {
    return display;
}

void Dashboard::setRotation(uint_fast8_t rotation) {
    display->setRotation(rotation);
#ifndef DISABLE_LVGL
    if (rotation == 1) rotation = 2;
    else if (rotation == 0) rotation = 3;
    else if (rotation == 2) rotation = 1;
    else if (rotation == 3) rotation = 4;
    LvglDriver::setRotation((lv_disp_rot_t)rotation);
#endif
}

void Dashboard::setBrightness(uint8_t brightness) {
    displayBrightness = brightness;
    display->setBrightness(brightness);
#ifdef ENABLE_SCREEN_SAVER
    disableScreenSaver();
#endif
}

#ifdef ENABLE_SCREEN_SAVER
void Dashboard::setScreenSaverTimeout(int timeoutMs) {
    screenSaverTimeoutMs = timeoutMs;
    disableScreenSaver();
}
void Dashboard::enableScreenSaver() {
    if (!isScreenSaverActive) {
        isScreenSaverActive = true;
        display->sleep();
        auto activity = getForegroundActivity();
        if (activity) {
            activity->pause();
        }
#ifndef DISABLE_LVGL
        LvglDriver::disableInput = true;
#endif
    }
}
void Dashboard::disableScreenSaver() {
    if (isScreenSaverActive) {
        isScreenSaverActive = false;
        display->wakeup();
        auto activity = getForegroundActivity();
        if (activity) {
            activity->resume();
        }
        // restore brightness
        display->setBrightness(displayBrightness);
    }
    lastTouchTs = millis();
}
#endif // ENABLE_SCREEN_SAVER

#endif // ENABLE_UI
