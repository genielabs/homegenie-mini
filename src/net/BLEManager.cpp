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

#include "BLEManager.h"

#ifndef DISABLE_BLE

#define BLEMANAGER_NS_PREFIX "Net::BLEManager"

#define SERVICE_UUID "033214d2-0ff0-4cba-814e-c5074c1ad00c"
#define CHARACTERISTIC_UUID "ac6744a7-77f3-43e9-b3c8-9955ac6bb0d4"

namespace Net {

    uint8_t devicesConnected = 0;
    uint8_t currentClients = 0;

    class CharacteristicCallbacks : public BLECharacteristicCallbacks {
        void onWrite(NimBLECharacteristic *characteristic) {
            Serial.println("onWrite");
            std::string value = characteristic->getValue();

            Serial.println(value.c_str());
        }

        void onRead(NimBLECharacteristic *characteristic) {
            Serial.println("onRead");
            characteristic->setValue("Hello");
        }
    };

    class ServerCallbacks : public BLEServerCallbacks {
        void onConnect(NimBLEServer *server) {
            Serial.println("Client connected");
            devicesConnected++;
            advertising->start();
        }

        void onDisconnect(NimBLEServer *server) {
            Serial.println("Client disconnected");
            devicesConnected--;
        }
    };

    BLEManager::BLEManager()
    {
        setLoopInterval(1000);
    }

    void BLEManager::begin() {

        NimBLEDevice::init(CONFIG_BUILTIN_MODULE_NAME);
        NimBLEServer *server = NimBLEDevice::createServer();

        server->setCallbacks(new ServerCallbacks());

        NimBLEService *service = server->createService(SERVICE_UUID);

        characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
/*        characteristic->createDescriptor("ABCD",
                                         NIMBLE_PROPERTY::READ |
                                         NIMBLE_PROPERTY::WRITE |
                                         NIMBLE_PROPERTY::WRITE_ENC,
                                         25);*/
        characteristic->setCallbacks(new CharacteristicCallbacks());

        service->start();

        advertising = NimBLEDevice::getAdvertising();
        advertising->addServiceUUID(SERVICE_UUID);
        advertising->setScanResponse(false);
        advertising->setMinPreferred(0x0);
        advertising->start();

        IO::Logger::info("|  ✔ BLE enabled");

    }

    void BLEManager::loop() {

        if (devicesConnected != currentClients) {
            currentClients = devicesConnected;
            IO::Logger::info("@%s [%s %d]", BLEMANAGER_NS_PREFIX, "Clients:", currentClients);
//            Serial.println("Notifying devices");
//            characteristic->setValue("Hello connected devices!");
//            characteristic->notify();
        }

    }

    void BLEManager::addHandler(IO::IIOEventReceiver* handler) {
        // TODO: ....
        //handler->onIOEvent(this, 0, 0);

        // TODO: ....
        // TODO: ....
        // TODO: ....


    }

} // Net

#endif