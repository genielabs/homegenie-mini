//
// Created by gene on 09/01/19.
//

#include "IOManager.h"

namespace IO {

    IOManager::IOManager() {
        // Instantiate the X10 Receiver Class
        ReceiverConfig x10ReceiverConfig = ReceiverConfig(CONFIG_RF_RX_PIN);
        x10Receiver = new Receiver(&x10ReceiverConfig, this);
        // X10 RF Receiver and Transmitter objects
        TransmitterConfig x10TransmitterConfig = TransmitterConfig(CONFIG_RF_TX_PIN);
        x10Transmitter = new Transmitter(&x10TransmitterConfig);
    }

    void IOManager::begin() {
        x10Receiver->begin();
        x10Transmitter->begin();
    }

    // TODO: move to an utility class (maybe static)
    /// Convert byte to hex string taking care of leading-zero
    /// \param b
    void IOManager::byteToHex(byte b) {
        if (b < 16) Serial.print("0");
        Serial.print(b, HEX);
    }

    /// Callback function that receives X10 RF messages via X10::Receiver instance
    /// \param type Type of message (eg. 0x20 = standard, 0x29 = security, ...)
    /// \param b0 Byte 1
    /// \param b1 Byte 2
    /// \param b2 Byte 3
    /// \param b3 Byte 4
    int IOManager::onX10RfDataReceived(uint8_t type, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
        if (x10Receiver->isEnabled()) {
            byteToHex(type);
            //Serial.print("-");
            byteToHex((b0));
            //Serial.print("-");
            byteToHex((b1));
            //Serial.print("-");
            byteToHex(b2);
            //Serial.print("-");
            byteToHex(b3);
            if (type == 0x29) {
                Serial.print("0000");
            }
            Serial.println("");
            //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            //delay(50);                         // wait for a second
            //digitalWrite(LED_BUILTIN, HIGH);
        }
    }

    Receiver *IOManager::getX10Receiver() {
        return x10Receiver;
    }

}