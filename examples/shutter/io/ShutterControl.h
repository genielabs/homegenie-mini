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

#include "drivers/ServoDriver.h"
#include "drivers/StepperDriver.h"

namespace IO { namespace Components {

    using namespace Service;

    class ShutterControl : public IIOEventSender {
    private:
        IShutterDriver* shutterDriver;
        String domain = IO::IOEventDomains::Automation_Components;
        String address = SERVO_MODULE_ADDRESS;
    public:
        ShutterControl() {
            shutterDriver = new ServoDriver();
            //shutterDriver = new StepperDriver();
            shutterDriver->eventSender = this;
        }

        void begin() override;

        void open();
        void close();
        void setLevel(float level);

        void calibrate(); // TODO:....
    };

}}

#endif //HOMEGENIE_MINI_SHUTTERCONTROL_H
