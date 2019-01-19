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

#include "HomeGenie.h"

namespace Service {

    HomeGenie::HomeGenie() {
    }

    void HomeGenie::begin() {
        netManager.begin();
        netManager.getHttpServer().addHandler(this);
        ioManager.begin();
        ioManager.setOnEventCallback(this);
    }

    void HomeGenie::loop() {
        Logger::verbose("%s loop() >> BEGIN", HOMEGENIEMINI_LOG_PREFIX);

        // HomeGenie-Mini Terminal CLI
        if(Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            auto apiCommand = ApiRequest::parse(cmd);
            if (apiCommand.Prefix.equals("api")) {
                if (api(&apiCommand)) {
                    Logger::info("%s =%s", HOMEGENIEMINI_LOG_PREFIX, apiCommand.Response.c_str());
                } else {
                    Logger::warn("%s =%s", HOMEGENIEMINI_LOG_PREFIX, apiCommand.Response.c_str());
                }
            }
        }

        // MQTT Events Queue (dequeue)
        for (int i = 0; i < eventsQueue.size(); i++) {
            // route event through MQTT
            auto m = eventsQueue.pop();
            netManager.getMQTTServer().broadcast(&m.sender, &m.details);
            // TODO: route event to the console as well
        }

        Logger::verbose("%s loop() << END", HOMEGENIEMINI_LOG_PREFIX);
    }

    IOManager& HomeGenie::getIOManager() {
        return ioManager;
    }

    // BEGIN IIOEventSender interface methods
    void HomeGenie::onIOEvent(IIOEventSender *sender, const unsigned char *eventPath, void *eventData) {
        String domain = String((char*)sender->getDomain());
        String address = String((char*)sender->getAddress());
        String event = String((char*)eventPath);
        /*
         * HomeAutomation.X10 events
         */
        if (domain == IOEventDomains::HomeAutomation_X10) {

            /*
             * X10 RF Receiver "Sensor.RawData" event
             */
            if (address == "RF"
                && event == IOEventPaths::Sensor_RawData
                && ioManager.getX10Receiver().isEnabled()) {
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
                Logger::info("%s [X10::RfReceiver] >> [%s%s%s%s%s%s]", HOMEGENIEMINI_LOG_PREFIX,
                     byteToHex(type).c_str(),
                     byteToHex((b0)).c_str(),
                     byteToHex((b1)).c_str(),
                     byteToHex(b2).c_str(),
                     byteToHex(b3).c_str(),
                     (type == 0x29) ? "0000" : ""
                );

                // Decode RF message data to X10Message class
                auto *decodedMessage = new X10Message();
                uint8_t encodedMessage[5]{type, b0, b1, b2, b3};
                X10Message::decodeCommand(encodedMessage, decodedMessage);

                // Convert enums to string
                String houseCode(house_code_to_char(decodedMessage->houseCode));
                String unitCode(unit_code_to_int(decodedMessage->unitCode));
                Logger::trace("%s %s%s %s", HOMEGENIEMINI_LOG_PREFIX, houseCode.c_str(), unitCode.c_str(),
                              cmd_code_to_str(decodedMessage->command));

                // NOTE: Calling `getMQTTServer().broadcast(..)` out of the loop() would cause crashing,
                // NOTE: so an `eventsQueue` is used to store messages that are then sent in the `loop()`
                // NOTE: method. Currently the queue will only hold one element but it can be used as a real
                // NOTE: queue by processing queued elements at every n-th loop() cycle (currently the queue
                // NOTE:  is processed at every cycle). (not sure if it would be of any use though)

                // MQTT Message Queue (enqueue)
                QueuedMessage m = QueuedMessage();
                m.sender = String("hg-mini/"+domain+"/" + houseCode + unitCode + "/event");
                m.details = String(R"({"Name":")")+IOEventPaths::Status_Level+ R"(", "Value": ")";
                switch (decodedMessage->command) {
                    case Command::CMD_ON:
                        m.details += "1\"}";
                        eventsQueue.add(m);
                        break;
                    case Command::CMD_OFF:
                        m.details += "0\"}";
                        eventsQueue.add(m);
                        break;
// TODO: Implement all X10 events also for Camera and Security
                }

                delete decodedMessage;

                // TODO: blink led ? (visible feedback)
                //digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
                //delay(10);                         // wait for a blink
                //digitalWrite(LED_BUILTIN, HIGH);
            }

        } else if (domain == IOEventDomains::HomeAutomation_HomeGenie) {

            // MQTT Message Queue (enqueue)
            QueuedMessage m = QueuedMessage();
            m.sender = String("hg-mini/"+domain+"/SYS/event");
            m.details = String(R"({"Name":"System.BytesFree","Value": ")")+String(((uint32_t &)eventData))+R"("})";
            eventsQueue.add(m);

        }







    }
    // END IIOEventSender

    // BEGIN RequestHandler interface methods
    bool HomeGenie::canHandle(HTTPMethod method, String uri) {
        return uri != NULL && uri.startsWith("/api/");
    }
    bool HomeGenie::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
        auto command = ApiRequest::parse(requestUri);
        if (api(&command)) {
            server.send(200, "application/json", command.Response);
        } else {
            server.send(400, "application/json", command.Response);
        }
        return true;
    }
    // END RequestHandler interface methods


    // TODO: move to an utility class (maybe static)
    /// Convert byte to hex string taking care of leading-zero
    /// \param b
    void HomeGenie::getBytes(const String &rawBytes, uint8_t *data) {
        uint len = rawBytes.length();
        char msg[len+1]; rawBytes.toCharArray(msg, len+1, 0);
        char tmp[3] = "00";
        len = (len / 2);
        for (uint i = 0; i < len; i++)
        {
            tmp[0] = msg[i * 2];
            tmp[1] = msg[(i * 2) + 1];
            data[i] = strtol(tmp, NULL, 16);
        }
    }
    String HomeGenie::byteToHex(byte b) {
        String formatted = String(b, HEX);
        if (b < 16) return "0"+formatted;
        return formatted;
    }

    bool HomeGenie::api(ApiRequest *command) {
        if (command->Domain.equals("HomeAutomation.X10")
            && command->Address.equals("RF")
            && command->Command.equals("Control.SendRaw")
        ) {

            uint8_t data[command->OptionsString.length() / 2]; getBytes(command->OptionsString, data);
            // Disable RfReceiver callbacks during transmission to prevent echo
            getIOManager().getX10Receiver().disable();
            getIOManager().getX10Transmitter().sendCommand(data, sizeof(data));
            getIOManager().getX10Receiver().enable();
            command->Response = R"({ "ResponseText": "OK" })";

        } else if (command->Domain.equals("HomeAutomation.Env")) {

            if (command->Address.equals("Light") && command->Command.equals("Sensor.GetValue")) {
                char response[1024];
                sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getLightSensor().getLightLevel() / 10.24F);
                command->Response = String(response);
            } else if (command->Address.equals("Temperature") && command->Command.equals("Sensor.GetValue")) {
                char response[1024];
                sprintf(response, R"({ "ResponseText": "%0.2f" })", getIOManager().getTemperatureSensor().getTemperature());
                command->Response = String(response);
            } else return false;

        } else return false;
        return true;
    }

}
