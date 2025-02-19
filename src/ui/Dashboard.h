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

#ifndef HOMEGENIE_MINI_DASHBOARD_H
#define HOMEGENIE_MINI_DASHBOARD_H

#include "defs.h"

#ifdef ENABLE_UI

#include <LovyanGFX.hpp>
#include <LinkedList.h>

#include "Task.h"
#include "PowerManager.h"
#include "io/Logger.h"

#include "drivers/RoundDisplay.h"

#include "Activity.h"

#define DASHBOARD_NS_PREFIX "UI::Dashboard"

using namespace IO;
using namespace UI;

class Dashboard: public Task, public GestureListener {

public:
    Dashboard(lgfx::LGFX_Device* display) {
//        setLoopInterval(10); // Task.h

        this->display = display;
        display->setColorDepth(16);
        gestureHelper = new GestureHelper(this);

        // apply display preferences
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, true);
        uint32_t displayRotation = preferences.getUInt(CONFIG_KEY_screen_rotation);
        preferences.end();
        display->setRotation(displayRotation);

        display->setBrightness(64);
        display->drawString("Hello World! =)", 72, 96);
    }

    void loop() override;

    Activity* getForegroundActivity();
    void setForegroundActivity(Activity*);
    Activity* getNextActivity();
    Activity* getPreviousActivity();
    void addActivity(Activity* activity);

    void onTap(PointerEvent e) override;
    void onSwipe(SwipeEvent e) override;
    void onPan(PanEvent e) override;
    void onTouch(PointerEvent e) override;
    void onRelease(PointerEvent e) override;

private:
    lgfx::LGFX_Device* display;
    LinkedList<Activity*> activityList;
    int currentActivityIndex = -1;
    Activity* nextActivity = nullptr;

    GestureHelper* gestureHelper;
    bool pointerDown = false;
    lgfx::touch_point_t lastTp;
    TouchPoint lastTouchPoint;
    TouchDirection swipeDirection;
};

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_DASHBOARD_H
