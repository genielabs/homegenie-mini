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
 *
 * Releases:
 * - 2024-01-06 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_SHUTTERCONTROL_H
#define HOMEGENIE_MINI_SHUTTERCONTROL_H

#include <HomeGenie.h>
#include <Utility.h>

#include "IShutterDriver.h"
#include "examples/shutter/io/drivers/ServoDriver.h"
// TODO: --> StepperDriver.h

#define SHUTTER_CONTROL_NS_PREFIX "IO::Components:ShutterControl"
#define SERVO_MODULE_ADDRESS "S1"

#define SHUTTER_COMMAND_NONE 0
#define SHUTTER_COMMAND_OPEN 1
#define SHUTTER_COMMAND_CLOSE 2

#define EVENT_EMIT_FREQUENCY 500

namespace IO { namespace Components {

    using namespace Service;

    class ShutterControl : Task, public IIOEventSender {
    private:
        IShutterDriver* shutterDriver;
        int lastCommand = 0;
        long lastCommandTs = 0;
        String domain = IO::IOEventDomains::Automation_Components;
        String address = SERVO_MODULE_ADDRESS;
        float currentLevel = 0;
        //
        float totalTimeSpanMs = 10000.0f;
        bool stopRequested = false;
        float stopTime;
    public:
        ShutterControl() {
            shutterDriver = new ServoDriver();
//            setLoopInterval(33); // see Task.h
        }

        void begin() override;
        void loop() override;

        void open();
        void close();
        void setLevel(float level);

        void calibrate(); // TODO:....
    };

}}

#endif //HOMEGENIE_MINI_SHUTTERCONTROL_H
