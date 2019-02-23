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
 * - 2019-02-03 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_P1PORT_H
#define HOMEGENIE_MINI_P1PORT_H

#include <Arduino.h>
#include <io/IOEvent.h>
#include <io/IOModule.h>
#include <io/IOEventDomains.h>
#include <io/IOEventPaths.h>
#include <io/Logger.h>

#define P1PORT_NS_PREFIX           "IO::GPIO::P1Port"

#define P1PORT_GPIO_COUNT           4
#define P1PORT_GPIO_LEVEL_MIN       0.0F
#define P1PORT_GPIO_LEVEL_MAX       100.0F

namespace IO { namespace GPIO {

    class P1Port: public IIOEventSender  {
    public:
        P1Port() {
            // IEventSender members
            domain = (const uint8_t*)IOEventDomains::HomeAutomation_HomeGenie;
        }
        void setOutput(uint8_t pinNumber, uint8_t value);
    };

}}


#endif //HOMEGENIE_MINI_P1PORT_H
