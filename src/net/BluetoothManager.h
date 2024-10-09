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
#include <BleSerial.h>
#endif

#ifndef DISABLE_BLUETOOTH_CLASSIC
#include <BluetoothSerial.h>
#endif

#include <Preferences.h>

namespace Net {

    class BluetoothManager : public Task, public  IO::IIOEventSender {
    public:
        BluetoothManager();
        void begin() override;
        void loop() override;
        void addHandler(IO::IIOEventReceiver* handler);

    private:
#ifndef DISABLE_BLUETOOTH_LE
        BleSerial* SerialBTLE = nullptr;
#endif
#ifndef DISABLE_BLUETOOTH_CLASSIC
        BluetoothSerial SerialBT;
#endif
        bool initialized = false;

        void handleMessage(String& message);
    };

} // Net

#endif //HOMEGENIE_MINI_BLEMANAGER_H

#endif
