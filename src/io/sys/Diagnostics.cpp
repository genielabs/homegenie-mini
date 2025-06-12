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
 * - 2019-01-19 Initial release
 *
 */

#include "Diagnostics.h"

namespace IO { namespace System {

    Diagnostics::Diagnostics() {
        // update interval
        setLoopInterval(DIAGNOSTICS_SAMPLING_RATE);
    }

    void Diagnostics::begin() {
        // Report system booting
        Logger::trace("@%s [%s %s]", DIAGNOSTICS_NS_PREFIX, IOEventPaths::System_Status, SystemStatus::BOOT);
        sendEvent(IOEventPaths::System_Status, (void*)&SystemStatus::BOOT, CString);
    }

    void Diagnostics::loop() {

        // Report free memory changes
        uint32_t freeMem = Utility::getFreeMem();
        if (currentFreeMemory != freeMem) {
            currentFreeMemory = freeMem;
            Logger::trace("@%s [%s %lu]", DIAGNOSTICS_NS_PREFIX, IOEventPaths::System_BytesFree, currentFreeMemory);
            sendEvent(IOEventPaths::System_BytesFree, &currentFreeMemory, UnsignedNumber);
#ifdef ESP32
            // Output DRAM info only to terminal
            String dramPath = IOEventPaths::System_BytesFree; dramPath.concat(".DRAM");
            multi_heap_info_t dram_info;
            heap_caps_get_info(&dram_info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
            Logger::trace("@%s [%s %lu]", DIAGNOSTICS_NS_PREFIX, dramPath.c_str(), dram_info.total_free_bytes);
#endif
        }

        // Report Wi-FI connection status
        bool connected = (ESP_WIFI_STATUS == WL_CONNECTED);
        if (connected != isWifiConnected) {
            isWifiConnected = connected;
            Logger::trace("@%s [%s %s]", DIAGNOSTICS_NS_PREFIX, IOEventPaths::System_Status, isWifiConnected ? SystemStatus::WIFI_CONNECTED : SystemStatus::WIFI_DISCONNECTED);
            sendEvent(IOEventPaths::System_Status, isWifiConnected ? (void*)&SystemStatus::WIFI_CONNECTED : (void*)&SystemStatus::WIFI_DISCONNECTED, CString);
        }

        // TODO: implement "WiFi.SignalStrength" --> WiFi.RSSI()

    }

}}
