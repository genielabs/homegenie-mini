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

#ifndef HOMEGENIE_MINI_ISHUTTERDRIVER_H
#define HOMEGENIE_MINI_ISHUTTERDRIVER_H

#define SHUTTER_CONTROL_NS_PREFIX "IO::Components:ShutterControl"
#define SERVO_MODULE_ADDRESS "S1"

#define SHUTTER_COMMAND_NONE 0
#define SHUTTER_COMMAND_OPEN 1
#define SHUTTER_COMMAND_CLOSE 2

#define EVENT_EMIT_FREQUENCY 500

namespace IO { namespace Components {
    class IShutterDriver {
    public:
        virtual void init() = 0;
        virtual void stop() = 0;
        virtual void open() = 0;
        virtual void close() = 0;
        virtual void level(float) = 0;
        virtual void calibrate() = 0;
        virtual void speed(float) = 0;
        IIOEventSender* eventSender = nullptr;
    };
}}

#endif //HOMEGENIE_MINI_ISHUTTERDRIVER_H
