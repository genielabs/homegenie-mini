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

    void IRTransceiverHandler::setReceiver(IRReceiver* r) {
        receiver = r;
    }

    void IRTransceiverHandler::setTransmitter(IRTransmitter* t) {
        this->transmitter = t;
        t->setListener(this);
    }

    void IRTransceiverHandler::init() {
        // Add IR sensor module
        auto irModule = new Module();
        irModule->domain = IO::IOEventDomains::HomeAutomation_RemoteControl;
        irModule->address = CONFIG_IR_MODULE_ADDRESS;
        irModule->type = ModuleApi::ModuleType::Sensor;
        irModule->name = CONFIG_IR_MODULE_ADDRESS; //TODO: CONFIG_IR_MODULE_NAME;

        // explicitly enable "scheduling" features for this module
        irModule->setProperty(Implements::Scheduling, "true");
        irModule->setProperty(Implements::Scheduling_ModuleEvents, "true");

        // add properties
        rawDataParameter = new ModuleParameter(IOEventPaths::Receiver_RawData);
        irModule->properties.add(rawDataParameter);
        rawDataParameter->value = "";

        moduleList.add(irModule);

        transmitter->begin();
        //receiver->begin(); <-- this is already done automatically by "IIOEventSender" parent class
        receiver->setModule(irModule);
    }

    bool IRTransceiverHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (command->Domain == (IOEventDomains::HomeAutomation_RemoteControl)
            && command->Address == CONFIG_IR_MODULE_ADDRESS) {

            if (command->Command == "Control.SendRaw") {

                auto commandString = command->getOption(0);
                auto sendRepeat = command->getOption(1).toInt();
                if (sendRepeat <= 0 || sendRepeat > 100) sendRepeat = 1; // default send repeat = 1
                auto sendDelay = command->getOption(2).toInt();
                if (sendDelay <= 0 || sendDelay > 10000) sendDelay = 1; // default delay = 1ms

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
                                      const char *eventPath, void *eventData, IOEventDataType dataType) {

        String event = String((char*)eventPath);
        /*
         * IR Receiver "Sensor.RawData" event
         */
        if (String(address) == CONFIG_IR_MODULE_ADDRESS && event == (IOEventPaths::Receiver_RawData) /*&& ioManager.getRCSReceiver().isEnabled()*/) {

            String stringData = *(String*)eventData;
            //Serial.println(stringData);
            Logger::info(":%s [RemoteControl::IR] >> [%s]", HOMEGENIEMINI_NS_PREFIX, stringData.c_str());

            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = std::make_shared<QueuedMessage>(domain, address, event.c_str(), stringData, eventData, dataType);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);

            // Update module parameter as well
            rawDataParameter->setData(eventData, dataType);
            rawDataParameter->setValue(m->value.c_str());

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
