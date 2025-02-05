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
 *
 * Releases:
 * - 2024-01-06 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_SHUTTERCONTROL_H
#define HOMEGENIE_MINI_SHUTTERCONTROL_H

#include <HomeGenie.h>
#include <Utility.h>

#include "../api/ShutterApi.h"

#include "drivers/ServoDriver.h"
#include "drivers/ServoEncoderDriver.h"
#include "drivers/StepperDriver.h"

namespace IO { namespace Components {

    using namespace Service;
    using namespace ShutterApi::Configuration;

    class ShutterControl : public IIOEventSender {
    private:
        int idx;
        IShutterDriver* shutterDriver = nullptr;
        bool calibrateMode = false;
    public:
        explicit ShutterControl(int index);

        void begin() override;

        void open();
        void close();
        void toggle();

        void setLevel(float);

        void setType(const char*);
        void calibrate();
        void configure(const char*, const char*);
    };

}}

#endif //HOMEGENIE_MINI_SHUTTERCONTROL_H
