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
 * - 2019-10-01 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_IOMANAGER_H
#define HOMEGENIE_MINI_IOMANAGER_H

#include <Arduino.h>

#include "Logger.h"
#include "rf/x10/Receiver.h"
#include "rf/x10/Transmitter.h"

#define CONFIG_RF_TX_PIN 4
#define CONFIG_RF_RX_PIN 5

namespace IO {

    using namespace X10;

    class IOManager : Receiver::X10RfDataReceivedCallback {
    public:
        IOManager();
        void begin();
        void onX10RfDataReceived(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

        Receiver& getX10Receiver(){ return *x10Receiver; };
        Transmitter& getX10Transmitter(){ return *x10Transmitter; };

    private:
        class X10ApiHandler;
        String byteToHex(byte b);
        // Instantiate the X10 Receiver Class
        ReceiverConfig *x10ReceiverConfig;
        Receiver *x10Receiver;
        // X10 RF Receiver and Transmitter objects
        TransmitterConfig *x10TransmitterConfig;
        Transmitter *x10Transmitter;
        // RF data received event handler
        void x10_RfReceivedCallback(uint8_t type, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3);
    };

}


#endif //HOMEGENIE_MINI_IOMANAGER_H
