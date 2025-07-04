/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

    X10Handler::X10Handler() {
        // RF module
        rfModule = new Module();
        rfModule->domain = IO::IOEventDomains::HomeAutomation_X10;
        rfModule->address = CONFIG_X10RF_MODULE_ADDRESS;
        rfModule->type = ModuleApi::ModuleType::Sensor;
        rfModule->name = "X10 RF"; //TODO: CONFIG_X10RF_MODULE_NAME;

        // explicitly enable "scheduling" features for this module
        rfModule->setProperty(Implements::Scheduling, "true");
        rfModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

        // set properties
        receiverRawData = new ModuleParameter(IOEventPaths::Receiver_RawData);
        rfModule->properties.add(receiverRawData);
        receiverCommand = new ModuleParameter(IOEventPaths::Receiver_Command);
        rfModule->properties.add(receiverCommand);

        moduleList.add(rfModule);

        auto hc = Config::getSetting("x10-hcode", "a");
        hc.toLowerCase();
        auto defaultHouseCode = hc.charAt(0);

        // Add 16 X10 modules for default house code
        // module address: "<house_code_a_p><unit_number_1_16>"
        // controlling these modules via API or app UI will result
        // in sending the X10 RF code mathing the command (e.g. A1 ON)
        for (int m = 0; m < UNIT_MAX; m++) {
            auto address = String((char)defaultHouseCode) + String(m+UNIT_MIN);
            address.toUpperCase();
            auto module = new Module();
            module->domain = IOEventDomains::HomeAutomation_X10;
            module->address = address;
            module->type = ModuleApi::ModuleType::Dimmer;
            module->setProperty(IOEventPaths::Status_Level, "0", 0, Number);
            moduleList.add(module);
        }

    }

    void X10Handler::init() {
        if (transmitter != nullptr) {
            transmitter->begin();
        }
    }


    bool X10Handler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {

        if (command->Domain == (IOEventDomains::HomeAutomation_X10)
            && command->Address == CONFIG_X10RF_MODULE_ADDRESS
            && command->Command == "Control.SendRaw") {

            auto commandString = command->getOption(0);
            auto sendRepeat = command->getOption(1).toInt();
            if (sendRepeat < 0 || sendRepeat > 100) sendRepeat = 0;
            if (sendRepeat == 0 && transmitter != nullptr) {
                sendRepeat = transmitter->
                        getConfiguration()->getSendRepeat();
            }
            auto sendDelay = command->getOption(2).toInt();
            if (sendDelay <= 0 || sendDelay > 10000) sendDelay = 1;

            uint8_t data[commandString.length() / 2];
            Utility::getBytes(commandString, data);

            if (receiver != nullptr) {
                receiver->disableMs(sendRepeat > 0 ? (sendRepeat * sendDelay) : 500);
            }
            if (transmitter != nullptr) {
                transmitter->sendCommand(&data[1], sizeof(data) - 1, sendRepeat, sendDelay);
            }
            responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            return true;
        } else if (command->Domain == (IOEventDomains::HomeAutomation_X10)) {
            uint8_t data[5];
            auto hu = command->Address; hu.toLowerCase();
            int h = (int)hu.charAt(0) - (int)HOUSE_MIN; // house code 0..15
            int u = hu.substring(1).toInt() - UNIT_MIN; // unit code 0..15
            //auto moduleStatus = &moduleList[h][u];

            auto x10Message = X10Message();
            x10Message.houseCode = HouseCodeLut[h];
            x10Message.unitCode = UnitCodeLut[u];

            uint8_t sendRepeat = 0; // fallback to default repeat (3)
            bool ignoreCommand = false;

            Module* module = getModule(command->Domain.c_str(), command->Address.c_str());
            if (module) {
                auto m = std::make_shared<QueuedMessage>(command->Domain, command->Address, (IOEventPaths::Status_Level), "");
                auto levelProperty = module->getProperty(IOEventPaths::Status_Level);

                if (command->Command == ControlApi::Control_On) {
                    x10Message.command = X10::Command::CMD_ON;
                    levelProperty->setValue("1");
                } else if (command->Command == ControlApi::Control_Off) {
                    x10Message.command = X10::Command::CMD_OFF;
                    levelProperty->setValue("0");
                } else if (command->Command == ControlApi::Control_Level) {
                    float level = command->getOption(0).toFloat() / 100.0f;
                    float prevLevel = levelProperty->value.toFloat();
                    sendRepeat = abs((level - prevLevel) / X10_DIM_BRIGHT_STEP);
                    if (level > prevLevel) {
                        x10Message.command = X10::Command::CMD_LIGHTS_BRIGHT;
                        levelProperty->value = String(prevLevel + (sendRepeat * X10_DIM_BRIGHT_STEP));
                        if (levelProperty->value.toFloat() > 1) levelProperty->value = "1";
                    } else if (level < prevLevel) {
                        x10Message.command = X10::Command::CMD_LIGHTS_DIM;
                        levelProperty->value = String(prevLevel - (sendRepeat * X10_DIM_BRIGHT_STEP));
                        if (levelProperty->value.toFloat() < 0) levelProperty->value = "0";
                    }
                    if (sendRepeat == 0) {
                        ignoreCommand = true;
                    } else {
                        sendRepeat += 2; // improve initial burst detection
                    }
                } else if (command->Command == ControlApi::Control_Toggle) {
                    if (levelProperty->value.toFloat() > 0) {
                        x10Message.command = X10::Command::CMD_OFF;
                        levelProperty->setValue("0");
                    } else {
                        x10Message.command = X10::Command::CMD_ON;
                        levelProperty->setValue("1");
                    }
                } else return false;

                m->value = levelProperty->value;
                HomeGenie::getInstance()->getEventRouter().signalEvent(m);
            }

            if (!ignoreCommand) {
                X10::X10Message::encodeCommand(&x10Message, data);
                if (receiver != nullptr) {
                    receiver->disableMs(500);
                }
                if (transmitter != nullptr) {
                    transmitter->sendCommand(&data[1], sizeof(data) - 1, sendRepeat, 1);
                }
            }
            responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            return true;
        }

        return false;
    }

    bool X10Handler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_X10);
    }

    bool X10Handler::handleEvent(IIOEventSender *sender,
                                 const char* domain, const char* address,
                                 const char *eventPath, void *eventData, IOEventDataType dataType) {
        auto module = getModule(domain, address);
        if (module) {
            String event = String((char*)eventPath);
            /*
             * X10 RF Receiver "Sensor.RawData" event
             */
            if (String(address) == CONFIG_X10RF_MODULE_ADDRESS && event == (IOEventPaths::Receiver_RawData) /*&& ioManager.getX10Receiver().isEnabled()*/) {
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

                //String rawDataString = Utility::byteToHex(type) + "-" + Utility::byteToHex((b0)) + "-" + Utility::byteToHex((b1)) + "-" + Utility::byteToHex(b2) + "-" + Utility::byteToHex(b3) + ((type == 0x29) ? "-00-00" : "");
                String rawDataString = Utility::byteToHex(type) + Utility::byteToHex((b0)) + Utility::byteToHex((b1)) + Utility::byteToHex(b2) + Utility::byteToHex(b3) + ((type == 0x29) ? "0000" : "");
                rawDataString.toUpperCase();
                Logger::info(":%s [X10::RFReceiver] >> [%s]", HOMEGENIEMINI_NS_PREFIX, rawDataString.c_str());

                // repeat can start only after "repeatDelay" ms
                uint16_t repeatDelay = 500;
                if (rawDataString.equals(lastEvent) && (millis() - lastEventTimestamp) < repeatDelay) {
                    eventTimestamp = millis();
                    return false;
                }
                if (!rawDataString.equals(lastEvent) || (millis() - eventTimestamp) >= repeatDelay / 2) {
                    lastEventTimestamp = millis();
                    lastEvent = rawDataString;
                }
                eventTimestamp = millis();

                // Decode RF message data to X10Message class
                auto *decodedMessage = new X10Message();
                uint8_t encodedMessage[5]{type, b0, b1, b2, b3};
                X10Message::decodeCommand(encodedMessage, decodedMessage);


                receiverRawData->setValue(rawDataString.c_str());

                auto m = std::make_shared<QueuedMessage>(domain, CONFIG_X10RF_MODULE_ADDRESS, IOEventPaths::Receiver_RawData, rawDataString);
                HomeGenie::getInstance()->getEventRouter().signalEvent(m);

                if (ledBlinkHandler) {
                    ledBlinkHandler(rawDataString.c_str());
                }


                // 0x20 = standard, 0x29 = security
                switch (type) {

                    case 0x20: {

                        // Convert enums to string
                        String houseCode(house_code_to_char(decodedMessage->houseCode));
                        String unitCode(unit_code_to_int(decodedMessage->unitCode));
                        if (unitCode == "0") unitCode = "";
                        const char* command = cmd_code_to_str(decodedMessage->command);
                        String commandString = (houseCode + unitCode + " " + command);

                        Logger::trace(":%s %s", HOMEGENIEMINI_NS_PREFIX, commandString.c_str());

                        receiverCommand->setValue(commandString.c_str());

                        auto msg = std::make_shared<QueuedMessage>(domain, CONFIG_X10RF_MODULE_ADDRESS, IOEventPaths::Receiver_Command, commandString);
                        HomeGenie::getInstance()->getEventRouter().signalEvent(msg);

                    } break;

                    case 0x29: {

                        int commandCode = (b2 << 8) + b3;
                        const char* command = cmd_code_to_str((Command)commandCode);
                        String commandString = (command);
                        String subtype = Utility::byteToHex((b0)) + Utility::byteToHex((b1));
                        subtype.toUpperCase();

                        Logger::trace(":%s S-%s %s", HOMEGENIEMINI_NS_PREFIX, subtype.c_str(), commandString.c_str());

                        receiverCommand->setValue(commandString.c_str());

                        auto msg = std::make_shared<QueuedMessage>(domain, CONFIG_X10RF_MODULE_ADDRESS, IOEventPaths::Receiver_Command, commandString);
                        HomeGenie::getInstance()->getEventRouter().signalEvent(msg);

                    } break;

                }

                delete decodedMessage;

                return true;
            }
        }
        return false;
    }

    Module* X10Handler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* X10Handler::getModuleList() {
        return &moduleList;
    }

    void X10Handler::setReceiver(X10RFReceiver *r) {
        receiver = r;
        r->setModule(rfModule);
    }

    void X10Handler::setTransmitter(X10RFTransmitter *t) {
        transmitter = t;
    }

    void X10Handler::setOnDataReady(std::function<void(const char*)> callback) {
        ledBlinkHandler = std::move(callback);
    }
}}
