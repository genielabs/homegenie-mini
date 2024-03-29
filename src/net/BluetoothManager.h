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
 * - 2023-12-08 Initial release
 *
 */
#include <Config.h>

#ifndef DISABLE_BLUETOOTH

#ifndef HOMEGENIE_MINI_BLEMANAGER_H
#define HOMEGENIE_MINI_BLEMANAGER_H

#include <Task.h>
#include <io/IOEvent.h>
#include <io/Logger.h>

#ifndef DISABLE_BLUETOOTH_LE
#include "lib/ESP32_BleSerial/src/BleSerial.h"
#endif

#ifndef DISABLE_BLUETOOTH_CLASSIC
#include <BluetoothSerial.h>
#endif
#ifndef DISABLE_PREFERENCES
#include <Preferences.h>
#endif

namespace Net {

    class BluetoothManager : public Task, public  IO::IIOEventSender {
    public:
        BluetoothManager();
        void begin() override;
        void loop() override;
        void addHandler(IO::IIOEventReceiver* handler);

    private:
#ifndef DISABLE_BLUETOOTH_LE
        BleSerial SerialBTLE;
#endif
#ifndef DISABLE_BLUETOOTH_CLASSIC
        BluetoothSerial SerialBT;
#endif
        bool initialized = false;
        unsigned long lastTs;
        uint8_t devicesConnected = 0;
        uint8_t currentClients = 0;

        void handleMessage(String& message);
    };

} // Net

#endif //HOMEGENIE_MINI_BLEMANAGER_H

#endif
