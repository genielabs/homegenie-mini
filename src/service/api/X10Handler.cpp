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
 * - 2019-01-28 Initial release
 *
 */

#include "X10Handler.h"

namespace Service { namespace API {

    bool X10Handler::handleRequest(HomeGenie &homeGenie, APIRequest *command) {

        if (command->Domain == IOEventDomains::HomeAutomation_X10
            && command->Address == "RF"
            && command->Command == "Control.SendRaw"
                ) {

            uint8_t data[command->OptionsString.length() / 2]; Utility::getBytes(command->OptionsString, data);
            // Disable RFReceiver callbacks during transmission to prevent echo
            noInterrupts();
            //getIOManager().getX10Receiver().disable();
            homeGenie.getIOManager().getX10Transmitter().sendCommand(data, sizeof(data));
            //getIOManager().getX10Receiver().enable();
            interrupts();
            command->Response = R"({ "ResponseText": "OK" })";

            return true;
        } else if (command->Domain == IOEventDomains::HomeAutomation_X10) {
            uint8_t data[5];
            auto hu = command->Address; hu.toLowerCase();
            auto x10Message = X10Message();
            x10Message.houseCode = HouseCodeLut[hu.charAt(0) - HOUSE_MIN];
            x10Message.unitCode = UnitCodeLut[hu.substring(1).toInt() - UNIT_MIN];

            QueuedMessage m = QueuedMessage(command->Domain, command->Address, IOEventPaths::Status_Level, "");
            if (command->Command == "Control.On") {
                x10Message.command = X10::Command::CMD_ON;
                m.value = "1";
            } else if (command->Command == "Control.Off") {
                x10Message.command = X10::Command::CMD_OFF;
                m.value = "0";
            }
            homeGenie.getEventRouter().signalEvent(m);

            X10::X10Message::encodeCommand(&x10Message, data);
            noInterrupts();
            //ioManager.getX10Receiver().disable();
            homeGenie.getIOManager().getX10Transmitter().sendCommand(&data[1], sizeof(data)-1);
            //ioManager.getX10Receiver().enable();
            interrupts();
            command->Response = R"({ "ResponseText": "OK" })";

            return true;
        }




        return false;
    }

    bool X10Handler::canHandleDomain(String &domain) {
        return domain == IO::IOEventDomains::HomeAutomation_X10;
    }

        bool X10Handler::handleEvent(HomeGenie &homeGenie, IIOEventSender *sender, const unsigned char *eventPath, void *eventData, IOEventDataType dataType) {

            String domain = String((char*)sender->getDomain());
            String address = String((char*)sender->getAddress());
            String event = String((char*)eventPath);
            /*
             * X10 RF Receiver "Sensor.RawData" event
             */
            if (address == "RF" && event == IOEventPaths::Sensor_RawData /*&& ioManager.getX10Receiver().isEnabled()*/) {
                // decode event data (X10 RF packet)
                auto data = ((uint8_t *) eventData);
                /// \param type Type of message (eg. 0x20 = standard, 0x29 = security, ...)
                /// \param b0 Byte 1
                /// \param b1 Byte 2
                /// \param b2 Byte 3
                /// \param b3 Byte 4
                uint8_t type = data[0];
                uint8_t b0 = data[1];
                uint8_t b1 = data[2];
                uint8_t b2 = data[3];
                uint8_t b3 = data[4];
                Logger::info(":%s [X10::RFReceiver] >> [%s%s%s%s%s%s]", HOMEGENIEMINI_NS_PREFIX,
                             Utility::byteToHex(type).c_str(),
                             Utility::byteToHex((b0)).c_str(),
                             Utility::byteToHex((b1)).c_str(),
                             Utility::byteToHex(b2).c_str(),
                             Utility::byteToHex(b3).c_str(),
                             (type == 0x29) ? "0000" : ""
                );

                // Decode RF message data to X10Message class
                auto *decodedMessage = new X10Message();
                uint8_t encodedMessage[5]{type, b0, b1, b2, b3};
                X10Message::decodeCommand(encodedMessage, decodedMessage);

                // Convert enums to string
                String houseCode(house_code_to_char(decodedMessage->houseCode));
                String unitCode(unit_code_to_int(decodedMessage->unitCode));
                Logger::trace(":%s %s%s %s", HOMEGENIEMINI_NS_PREFIX, houseCode.c_str(), unitCode.c_str(),
                              cmd_code_to_str(decodedMessage->command));

                // NOTE: Calling `getMQTTServer().broadcast(..)` out of the loop() would cause crashing,
                // NOTE: so an `eventsQueue` is used to store messages that are then sent in the `loop()`
                // NOTE: method. Currently the queue will only hold one element but it can be used as a real
                // NOTE: queue by processing queued elements at every n-th loop() cycle (currently the queue
                // NOTE:  is processed at every cycle). (not sure if it would be of any use though)

                // MQTT Message Queue (enqueue)
                QueuedMessage m = QueuedMessage(domain, houseCode + unitCode, IOEventPaths::Status_Level, "");
                switch (decodedMessage->command) {
                    case Command::CMD_ON:
                        m.value = "1";
                        homeGenie.getEventRouter().signalEvent(m);
                        break;
                    case Command::CMD_OFF:
                        m.value = "0";
                        homeGenie.getEventRouter().signalEvent(m);
                        break;
// TODO: Implement all X10 events also for Camera and Security
                }

                delete decodedMessage;

                // TODO: blink led ? (visible feedback)
                //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
                //delay(10);                         // wait for a blink
                //digitalWrite(LED_BUILTIN, HIGH);

                return true;
            }


            return false;
        }

    }}
