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
 * - 2020-01-18 Initial release
 *
 */

#include "RCSwitchHandler.h"

namespace Service { namespace API {

        RCSwitchHandler::RCSwitchHandler() {
            // HomeGenie Mini module
            auto rfModule = new Module();
            rfModule->domain = IOEventDomains::HomeAutomation_RemoteControl;
            rfModule->address = CONFIG_RCSwitchRF_MODULE_ADDRESS;
            rfModule->type = ModuleApi::ModuleType::Sensor;
            rfModule->name = "RC RF"; //TODO: CONFIG_RCSwitchRF_MODULE_NAME;

            // explicitly enable "scheduling" features for this module
            rfModule->setProperty(Implements::Scheduling, "true");
            rfModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

            // add properties
            rawDataParameter = new ModuleParameter(IOEventPaths::Receiver_RawData);
            rfModule->properties.add(rawDataParameter);
            rawDataParameter->value = "";

            moduleList.add(rfModule);
        }

        void RCSwitchHandler::init() {
            transmitter->begin();
        }

        bool RCSwitchHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (command->Domain == (IOEventDomains::HomeAutomation_RemoteControl)
            && command->Address == CONFIG_RCSwitchRF_MODULE_ADDRESS
            && command->Command == "Control.SendRaw"
            && command->OptionsString.length() > 2) {

            auto sendRepeat = command->getOption(1).toInt();
            if (sendRepeat <= 0 || sendRepeat > 100) sendRepeat = 1; // default send repeat = 1
            auto sendDelay = command->getOption(2).toInt();
            if (sendDelay <= 0 || sendDelay > 10000) sendDelay = 1; // default delay = 1ms

            // parse type info
            auto hexType = command->OptionsString.substring(0, 2);
            long type = strtol(hexType.c_str(), nullptr, 16);
            auto bitLength = type >> 3;
            auto bitProtocol = type & 0x0F; // actually not used

            // parse long data from options string
            auto dataString = command->OptionsString.substring(2);
            long data = strtol(dataString.c_str(), nullptr, 16);

            receiver->disable();
            transmitter->sendCommand(data, bitLength, sendRepeat, sendDelay);
            receiver->enable();

            responseCallback->writeAll(ApiHandlerResponseStatus::OK);

            return true;
        }
        return false;
    }

    bool RCSwitchHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_RemoteControl);
    }

    bool RCSwitchHandler::handleEvent(IIOEventSender *sender,
                                      const char* domain, const char* address,
                                      const char *eventPath, void *eventData, IOEventDataType dataType) {

        auto module = getModule(domain, address);
        if (module) {
            String event = String((char*)eventPath);
            /*
             * X10 RF Receiver "Sensor.RawData" event
             */
            if (String(address) == CONFIG_RCSwitchRF_MODULE_ADDRESS && event == (IOEventPaths::Receiver_RawData) /*&& ioManager.getRCSReceiver().isEnabled()*/) {
                // decode event data (X10 RF packet)
                auto data = ((uint8_t *) eventData);

                uint8_t type = data[0];
                uint8_t b0 = data[1];
                uint8_t b1 = data[2];
                uint8_t b2 = data[3];

                //String rawDataString = Utility::byteToHex(type) + "-" + Utility::byteToHex((b0)) + "-" + Utility::byteToHex((b1)) + "-" + Utility::byteToHex(b2) + "-" + Utility::byteToHex(b3) + ((type == 0x29) ? "-00-00" : "");
                String rawDataString = Utility::byteToHex(type) + Utility::byteToHex((b0)) + Utility::byteToHex((b1)) + Utility::byteToHex(b2);
                rawDataString.toUpperCase();
                Logger::info(":%s [RCSwitch::RFReceiver] >> [%s]", HOMEGENIEMINI_NS_PREFIX, rawDataString.c_str());

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

                auto bitLength = type >> 3;
                auto bitProtocol = type & 0x0F;
//Serial.printf("\nBit length = %d, Protocol = %d\n\n", bitLength, bitProtocol);

                rawDataParameter->setValue(rawDataString.c_str());

                auto m = std::make_shared<QueuedMessage>(domain, CONFIG_RCSwitchRF_MODULE_ADDRESS, IOEventPaths::Receiver_RawData, rawDataString);
                HomeGenie::getInstance()->getEventRouter().signalEvent(m);

                if (ledBlinkHandler) {
                    ledBlinkHandler(rawDataString.c_str());
                }

                return true;
            }
        }

        return false;

    }

    Module* RCSwitchHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* RCSwitchHandler::getModuleList() {
        return &moduleList;
    }

    void RCSwitchHandler::setOnDataReady(std::function<void(const char*)> callback) {
        ledBlinkHandler = std::move(callback);
    }

    void RCSwitchHandler::setTransmitter(RFTransmitter *transmitter) {
        this->transmitter = transmitter;
    }

    void RCSwitchHandler::setReceiver(RFReceiver *receiver) {
        this->receiver = receiver;
        receiver->setModule(getModule(IOEventDomains::HomeAutomation_RemoteControl, CONFIG_RCSwitchRF_MODULE_ADDRESS));
    }

}}
