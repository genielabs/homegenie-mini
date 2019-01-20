/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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
 * - 2019-12-01 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_LIGHTSENSOR_H
#define HOMEGENIE_MINI_LIGHTSENSOR_H

#include <Task.h>
#include <io/Logger.h>
#include <io/IOEventPaths.h>
#include <io/IOEvent.h>
#include <io/IOEventDomains.h>

#define LIGHTSENSOR_NS_PREFIX           "IO::Env::LightSensor"
#define LIGHTSENSOR_SAMPLING_RATE       5000L

namespace IO { namespace Env {

    class LightSensor : Task, public IIOEventSender {
    public:
        LightSensor() {
            setLoopInterval(LIGHTSENSOR_SAMPLING_RATE);
            // IEventSender members
            domain = (uint8_t *)IOEventDomains::HomeAutomation_HomeGenie;
            address = (uint8_t *)"mini";
        }
        void begin();
        void loop();
        void setInputPin(uint8_t number);
        uint16_t getLightLevel();
    private:
        uint8_t inputPin = 0;
        uint16_t currentLevel = 0;
    };

}}

#endif //HOMEGENIE_MINI_LIGHTSENSOR_H
