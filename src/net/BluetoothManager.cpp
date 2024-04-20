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

#include "BluetoothManager.h"

#ifndef DISABLE_BLUETOOTH

#define BLEMANAGER_NS_PREFIX "Net::BluetoothManager"

namespace Net {

    BluetoothManager::BluetoothManager()
    {
//        setLoopInterval(100); // Task.h
    }

    void BluetoothManager::begin() {
        if (Config::isDeviceConfigured()) return;

#ifndef DISABLE_BLUETOOTH_CLASSIC
        SerialBT.begin(CONFIG_BUILTIN_MODULE_NAME);
        IO::Logger::info("|  ✔ Bluetooth enabled");
        initialized = true;
#endif

#ifndef DISABLE_BLUETOOTH_LE
        // Get unit MAC address
        //esp_read_mac(unitMACAddress, ESP_MAC_WIFI_STA);
        // Convert MAC address to Bluetooth MAC (add 2): https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
        //unitMACAddress[5] += 2;
        //Create device name
        //sprintf(deviceName, CONFIG_BUILTIN_MODULE_NAME, unitMACAddress[4], unitMACAddress[5]);
        //Init BLE Serial
        String bleName = String(CONFIG_BUILTIN_MODULE_NAME) + String("-LE");
        SerialBTLE.begin(bleName.c_str());
        SerialBTLE.setTimeout(10);
        IO::Logger::info("|  ✔ Bluetooth LE enabled");
        initialized = true;
#endif

    }

    void BluetoothManager::loop() {
        if (!initialized) return;

#ifndef DISABLE_BLUETOOTH_LE
        if (SerialBTLE.available()) {
            String message = SerialBTLE.readStringUntil('\n');
            if (message != nullptr) {
                handleMessage(message);
            }
        }
#endif

#ifndef DISABLE_BLUETOOTH_CLASSIC
        if (SerialBT.available()) {
            String message = SerialBT.readStringUntil('\n');
            if (message != nullptr) {
                handleMessage(message);
            }
        }
#endif

    }

    void BluetoothManager::addHandler(IO::IIOEventReceiver* handler) {
        // TODO: ....
        //handler->onIOEvent(this, 0, 0);
        // TODO: ....
        // TODO: ....
        // TODO: ....
    }

    void BluetoothManager::handleMessage(String& message) {
#ifndef DISABLE_PREFERENCES
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, false);

        if (message.startsWith("#CONFIG:device-name ")) {
            preferences.putString(CONFIG_KEY_device_name, message.substring(20));
        }
        if (message.startsWith("#CONFIG:wifi-ssid ")) {
            preferences.putString(CONFIG_KEY_wifi_ssid, message.substring(18));
        }
        if (message.startsWith("#CONFIG:wifi-password ")) {
            preferences.putString(CONFIG_KEY_wifi_password, message.substring(22));
            // reset system mode if it was previously forced to "config"
            preferences.putString(CONFIG_KEY_system_mode, "");
        }
        if (message.startsWith("#CONFIG:system-time ")) {
            String time = message.substring(20);
            long seconds = time.substring(0, time.length() - 3).toInt();
            long ms = time.substring(time.length() - 3).toInt();
            Config::getRTC()->setTime(seconds, ms);
        }
        if (message.startsWith("#CONFIG:system-zone ")) {
            int zoneMs = message.substring(20).toInt();
            preferences.putInt(CONFIG_KEY_system_zone, zoneMs);
            Config::setTimeZone(zoneMs);
        }
        preferences.end();

        if (message.equals("#RESET")) {
#ifndef DISABLE_BLUETOOTH_LE
            SerialBTLE.end();
#endif
#ifndef DISABLE_BLUETOOTH_CLASSIC
            SerialBT.disconnect();
            SerialBT.end();
#endif
            IO::Logger::info("RESET!");
            delay(50);
            esp_restart();
        }
#endif //DISABLE_PREFERENCES
    }

} // Net

#endif