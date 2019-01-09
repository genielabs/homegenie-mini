//
// Created by gene on 09/01/19.
//

#ifndef HOMEGENIE_MINI_IOMANAGER_H
#define HOMEGENIE_MINI_IOMANAGER_H

#include <Arduino.h>

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
        Transmitter* getX10Transmitter(){ return x10Transmitter; };
        int onX10RfDataReceived(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

        Receiver *getX10Receiver();

    private:
        class X10ApiHandler;
        void byteToHex(byte b);
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
