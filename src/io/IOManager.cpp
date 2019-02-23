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
 * - 2019-01-10 Initial release
 *
 */

#include <io/IOManager.h>

namespace IO {

    IOManager::IOManager() {
        systemDiagnostics = new System::Diagnostics();
        systemDiagnostics->setEventReceiver(this);
        // Instantiate the X10 RFReceiver Class
        RFReceiverConfig x10ReceiverConfig = RFReceiverConfig(CONFIG_RF_RX_PIN);
        x10Receiver = new RFReceiver(&x10ReceiverConfig);
        x10Receiver->setEventReceiver(this);
        // X10 RF RFReceiver and RFTransmitter objects
        RFTransmitterConfig x10TransmitterConfig = RFTransmitterConfig(CONFIG_RF_TX_PIN);
        x10Transmitter = new RFTransmitter(&x10TransmitterConfig);
        // DS18B20 Temperature Sensor
        temperatureSensor = new DS18B20();
        temperatureSensor->setEventReceiver(this);
        // Light Sensor
        lightSensor = new LightSensor();
        lightSensor->setEventReceiver(this);
        // P1 expansion port
        p1Port = new P1Port();
        p1Port->setEventReceiver(this);
    }

    void IOManager::begin() {
        x10Receiver->begin();
        x10Transmitter->begin();
        temperatureSensor->begin();
        lightSensor->begin();
    }

    void IOManager::setOnEventCallback(IIOEventReceiver *callback) {
        ioEventCallback = callback;
    }

    void IOManager::onIOEvent(IIOEventSender *sender, const unsigned char *eventPath, void *eventData, IOEventDataType dataType) {
        // route event to HomeGenie
        ioEventCallback->onIOEvent(sender, eventPath, eventData, dataType);
    }

}