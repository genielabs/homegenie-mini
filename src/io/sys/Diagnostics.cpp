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
 * - 2019-01-19 Initial release
 *
 */

#include "Diagnostics.h"

namespace IO { namespace System {

    Diagnostics::Diagnostics() {
        // update interval
        setLoopInterval(DIAGNOSTICS_SAMPLING_RATE);
    }

    void Diagnostics::begin() { }

    void Diagnostics::loop() {
        uint32_t freeMem = Utility::getFreeMem();
        if (currentFreeMemory != freeMem) {
            Logger::trace("@%s [%s %lu]", DIAGNOSTICS_NS_PREFIX, IOEventPaths::System_BytesFree, freeMem, UnsignedNumber);
            sendEvent(domain, address, IOEventPaths::System_BytesFree, &freeMem, UnsignedNumber);
            currentFreeMemory = freeMem;
        }
    }

}}
