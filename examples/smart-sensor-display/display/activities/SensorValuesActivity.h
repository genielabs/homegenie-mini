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

#ifndef HOMEGENIE_MINI_SENSORVALUESACTIVITY_H
#define HOMEGENIE_MINI_SENSORVALUESACTIVITY_H

#include <HomeGenie.h>
#include <LovyanGFX.hpp>
#include <ui/Activity.h>

#include "../bitmaps/Background_1.h"

using namespace Service;
using namespace UI;

class SensorValuesActivity: public Activity {

public:
    explicit SensorValuesActivity(Module* sensorModule) {
        setDrawInterval(1000);  // Task.h - 1000ms loop frequency
        this->sensorModule = sensorModule;
    }

    void attach(LGFX_Device* displayDevice) override;

    void onResume() override;
    void onPause() override;
    void onDraw() override;

private:
    Module* sensorModule;

};

#endif //HOMEGENIE_MINI_SENSORVALUESACTIVITY_H
