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

#include <io/rf/x10/X10Message.h>
#include <service/ApiRequest.h>
#include "IOManager.h"

namespace IO {

    IOManager::IOManager() {
        // Instantiate the X10 RfReceiver Class
        RfReceiverConfig x10ReceiverConfig = RfReceiverConfig(CONFIG_RF_RX_PIN);
        x10Receiver = new RfReceiver(&x10ReceiverConfig, this);
        // X10 RF RfReceiver and RfTransmitter objects
        RfTransmitterConfig x10TransmitterConfig = RfTransmitterConfig(CONFIG_RF_TX_PIN);
        x10Transmitter = new RfTransmitter(&x10TransmitterConfig);
        // DS18B20 Temperature Sensor
        temperatureSensor = new DS18B20();
        // Light Sensor
        lightSensor = new LightSensor();
    }

    void IOManager::begin() {
        x10Receiver->begin();
        x10Transmitter->begin();
        temperatureSensor->begin();
        lightSensor->begin();
    }

    // TODO: move to an utility class (maybe static)
    /// Convert byte to hex string taking care of leading-zero
    /// \param b
    String IOManager::byteToHex(byte b) {
        String formatted = String(b, HEX);
        if (b < 16) return "0"+formatted;
        return formatted;
    }

    /// Callback function that receives X10 RF messages via X10::Receiver instance
    /// \param type Type of message (eg. 0x20 = standard, 0x29 = security, ...)
    /// \param b0 Byte 1
    /// \param b1 Byte 2
    /// \param b2 Byte 3
    /// \param b3 Byte 4
    void IOManager::onX10RfDataReceived(uint8_t type, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
        if (x10Receiver->isEnabled()) {
            Logger::info("%s [X10::RfReceiver] >> [%s%s%s%s%s%s]", IOMANAGER_LOG_PREFIX,
                 byteToHex(type).c_str(),
                 byteToHex((b0)).c_str(),
                 byteToHex((b1)).c_str(),
                 byteToHex(b2).c_str(),
                 byteToHex(b3).c_str(),
                 (type == 0x29) ? "0000" : ""
            );

            auto *decodedMessage = new X10Message();
            uint8_t encodedMessage[5]{ type, b0, b1, b2, b3 };
            X10Message::decodeCommand(encodedMessage, decodedMessage);
            String houseCode(house_code_to_char(decodedMessage->houseCode));
            String unitCode(unit_code_to_int(decodedMessage->unitCode));
            Logger::trace("%s %s%s %s", IOMANAGER_LOG_PREFIX, houseCode.c_str(), unitCode.c_str(), cmd_code_to_str(decodedMessage->command));

            if (ioEventCallback != NULL) {

                auto sender = String("hg-mini/HomeAutomation.X10/"+houseCode+unitCode+"/event");
                String event = R"({"Name":"Status.Level","Value":")";
                switch (decodedMessage->command) {
                    case Command::CMD_ON:
                        event += "1\"}";
                        ioEventCallback->onIOEvent(&sender, &event);
                        break;
                    case Command::CMD_OFF:
                        event += "0\"}";
                        ioEventCallback->onIOEvent(&sender, &event);
                        break;
                }

            }
            delete decodedMessage;

            // TODO: blink led ? (visible feedback)
            // TODO: this might conflict with DS18B20 temperature reading (same GPIO)
            //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
            //delay(10);                         // wait for a blink
            //digitalWrite(LED_BUILTIN, HIGH);
        }
    }

    void IOManager::setOnEventCallback(IOManager::IOEventCallback *callback) {
        ioEventCallback = callback;
    }

}