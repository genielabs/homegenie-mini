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

#ifndef HOMEGENIE_MINI_IOMANAGER_H
#define HOMEGENIE_MINI_IOMANAGER_H

#include <Arduino.h>

#include "Logger.h"

#include <io/IOEvent.h>
#include <io/IOEventDomains.h>
#include <io/IOEventPaths.h>
#include <io/env/DS18B20.h>
#include <io/env/LightSensor.h>
#include <io/gpio/P1Port.h>
#include <io/rf/x10/RFReceiver.h>
#include <io/rf/x10/RFTransmitter.h>
#include <io/rf/x10/X10Message.h>
#include <io/sys/Diagnostics.h>
#include <service/api/APIRequest.h>

#define CONFIG_RF_RX_PIN D1 // 5
#define CONFIG_RF_TX_PIN D2 // 4

namespace IO {

    using namespace Env;
    using namespace GPIO;
    using namespace X10;

    class IOManager : IIOEventReceiver {
    public:

        IOManager();

        void begin();

        // IIOEventReceiver interface
        void onIOEvent(IIOEventSender *, const uint8_t *, void *, IOEventDataType);

        void setOnEventCallback(IIOEventReceiver *);

        RFReceiver getX10Receiver(){ return *x10Receiver; }
        RFTransmitter getX10Transmitter(){ return *x10Transmitter; }
        DS18B20 getTemperatureSensor(){ return *temperatureSensor; }
        LightSensor getLightSensor(){ return *lightSensor; }
        P1Port getExpansionPort(){ return *p1Port; }

    private:
        // Diagnostics
        System::Diagnostics *systemDiagnostics;
        //class X10ApiHandler;
        IIOEventReceiver *ioEventCallback;
        // Instantiate the X10 RFReceiver Class
        RFReceiverConfig *x10ReceiverConfig;
        RFReceiver *x10Receiver;
        // X10 RF RFReceiver and RFTransmitter objects
        RFTransmitterConfig *x10TransmitterConfig;
        RFTransmitter *x10Transmitter;
        // DS18B20 Temperature sensor
        DS18B20 *temperatureSensor;
        // Light Sensor / PhotoResistor
        LightSensor *lightSensor;
        // P1Port GPIOs
        P1Port *p1Port;
    };

}

#endif //HOMEGENIE_MINI_IOMANAGER_H
