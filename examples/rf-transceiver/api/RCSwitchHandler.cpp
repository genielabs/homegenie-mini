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
 *
 * Releases:
 * - 2020-01-18 Initial release
 *
 */

#include "RCSwitchHandler.h"

namespace Service { namespace API {

        RCSwitchHandler::RCSwitchHandler(RFTransmitter* transmitter) {
            this->transmitter = transmitter;

            // HomeGenie Mini module
            auto rfModule = new Module();
            rfModule->domain = IO::IOEventDomains::HomeAutomation_RemoteControl;
            rfModule->address = CONFIG_RCSwitchRF_MODULE_ADDRESS;
            rfModule->type = "Sensor";
            rfModule->name = "RF"; //TODO: CONFIG_RCSwitchRF_MODULE_NAME;
            // explicitly enable "scheduling" features for this module
            rfModule->setProperty("Widget.Implements.Scheduling", "1");
            // add properties
            rfModule->setProperty(IOEventPaths::Receiver_RawData, "");

            moduleList.add(rfModule);

        }

        void RCSwitchHandler::init() {
            transmitter->begin();
        }

        bool RCSwitchHandler::handleRequest(APIRequest *command, ResponseCallback* responseCallback) {
        if (command->Domain == (IOEventDomains::HomeAutomation_RemoteControl)
            && command->Address == CONFIG_RCSwitchRF_MODULE_ADDRESS
            && command->Command == "Control.SendRaw") {

            // parse long data from options string
            long data = atol(command->OptionsString.c_str());
            transmitter->sendCommand(data, 24, 1, 0);
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

        String event = String((char*)eventPath);
        /*
         * RCS RF Receiver "Sensor.RawData" event
         */
        if (String(address) == CONFIG_RCSwitchRF_MODULE_ADDRESS && event == (IOEventPaths::Receiver_RawData) /*&& ioManager.getRCSReceiver().isEnabled()*/) {
            // TODO: ...
            return true;
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

}}
