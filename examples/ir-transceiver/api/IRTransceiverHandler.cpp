/*
 * HomeGenie-Mini (c) 2018-2024 G-Labs
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
 */

#include "IRTransceiverHandler.h"

namespace Service { namespace API {

    IRTransceiverHandler::IRTransceiverHandler(IRTransmitter* transmitter, IRReceiver* receiver) {
        this->transmitter = transmitter;
        transmitter->setListener(this);
        this->receiver = receiver;

        auto domain = IO::IOEventDomains::HomeAutomation_RemoteControl;
        // HomeGenie Mini module
        auto rfModule = new Module();
        rfModule->domain = domain;
        rfModule->address = CONFIG_IR_MODULE_ADDRESS;
        rfModule->type = "Sensor";
        rfModule->name = CONFIG_IR_MODULE_ADDRESS; //TODO: CONFIG_IR_MODULE_NAME;
        // add properties
        auto propRawData = new ModuleParameter(IOEventPaths::Receiver_RawData);
        rfModule->properties.add(propRawData);

        moduleList.add(rfModule);

        receiver->setModule(rfModule);
    }

    void IRTransceiverHandler::init() {
        transmitter->begin();
        //receiver->begin(); <-- this is already done automatically by "IIOEventSender" parent class
    }

    bool IRTransceiverHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (command->Domain == (IOEventDomains::HomeAutomation_RemoteControl)
            && command->Address == CONFIG_IR_MODULE_ADDRESS) {

            if (command->Command == "Control.SendRaw") {

                auto commandString = command->getOption(0);
                auto sendRepeat = command->getOption(1).toInt();
                if (sendRepeat <= 0) sendRepeat = 1; // send at least once
                auto sendDelay = command->getOption(2).toInt();

                transmitter->sendCommand(commandString, sendRepeat, sendDelay);

            }

            responseCallback->writeAll(ApiHandlerResponseStatus::OK);
            return true;
        }
        return false;
    }

    bool IRTransceiverHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_RemoteControl);
    }

    bool IRTransceiverHandler::handleEvent(IIOEventSender *sender,
                                      const char* domain, const char* address,
                                      const unsigned char *eventPath, void *eventData, IOEventDataType dataType) {

        String event = String((char*)eventPath);
        /*
         * IR Receiver "Sensor.RawData" event
         */
        if (String(address) == CONFIG_IR_MODULE_ADDRESS && event == (IOEventPaths::Receiver_RawData) /*&& ioManager.getRCSReceiver().isEnabled()*/) {

            String stringData = *(String*)eventData;
            //Serial.println(stringData);
            Logger::info(":%s [RemoteControl::IR] >> [%s]", HOMEGENIEMINI_NS_PREFIX, stringData.c_str());

            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = QueuedMessage(domain, address, event.c_str(), stringData, eventData, dataType);
//            module->setProperty(event, m.value, eventData, dataType);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);

            return true;
        }

        return false;
    }

    Module* IRTransceiverHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }

    LinkedList<Module*>* IRTransceiverHandler::getModuleList() {
        return &moduleList;
    }

    void IRTransceiverHandler::onSendStart() {
        receiver->enabled(false);
    }
    void IRTransceiverHandler::onSendComplete() {
        receiver->enabled(true);
    }

}}
