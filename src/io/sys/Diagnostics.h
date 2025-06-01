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

#ifndef HOMEGENIE_MINI_DIAGNOSTICS_H
#define HOMEGENIE_MINI_DIAGNOSTICS_H

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

#include <io/Logger.h>

#include "Task.h"
#include "io/IOEvent.h"
#include "io/IOEventDomains.h"
#include "io/IOEventPaths.h"
#include "Utility.h"

#define DIAGNOSTICS_NS_PREFIX          "IO::Sys::Diagnostics"
#define DIAGNOSTICS_SAMPLING_RATE       5000L

namespace IO { namespace System {

    namespace SystemStatus {
        const char BOOT[]              PROGMEM = "boot";
        const char DEEP_SLEEP[]        PROGMEM = "deep_sleep";
        const char SLEEP[]             PROGMEM = "sleep";
        const char AWAKE[]             PROGMEM = "awake";
        const char WIFI_CONNECTED[]    PROGMEM = "wifi_connected";
        const char WIFI_DISCONNECTED[] PROGMEM = "wifi_disconnected";
    };

    class Diagnostics : Task, public IIOEventSender {
    public:
        Diagnostics();
        void begin() override;
        void loop() override;

        void setModule(Module* m) override {
            IIOEventSender::setModule(m);
            m->setProperty(IOEventPaths::System_BytesFree, "0");
            m->setProperty(IOEventPaths::System_Status, "");
        }

    private:
        const char* domain = IOEventDomains::HomeAutomation_HomeGenie;
        const char* address = CONFIG_BUILTIN_MODULE_ADDRESS;
        uint32_t currentFreeMemory;
        int isWifiConnected = (ESP_WIFI_STATUS == WL_CONNECTED);
    };

}}


#endif //HOMEGENIE_MINI_DIAGNOSTICS_H
