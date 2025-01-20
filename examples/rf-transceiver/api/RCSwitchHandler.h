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

#ifndef HOMEGENIE_MINI_RCSWITCHHANDLER_H
#define HOMEGENIE_MINI_RCSWITCHHANDLER_H

#include <HomeGenie.h>

#include "../configuration.h"
#include "../io/RFReceiver.h"
#include "../io/RFTransmitter.h"

namespace Service { namespace API {

    using namespace IO::RCS;

    class RCSwitchHandler : public APIHandler {
    private:
        LinkedList<Module*> moduleList;
        RFReceiver* receiver;
        RFTransmitter* transmitter;
        ModuleParameter* rawDataParameter{};
        unsigned long eventTimestamp = 0;
        String lastEvent;
        unsigned long lastEventTimestamp = 0;
        std::function<void(const char*)> ledBlinkHandler = nullptr;

    public:
        RCSwitchHandler();

        void setTransmitter(RFTransmitter* transmitter);
        void setReceiver(RFReceiver* receiver);

        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest *request, ResponseCallback* responseCallback) override;
        bool handleEvent(IIOEventSender *sender,
                         const char* domain, const char* address,
                         const char *eventPath, void *eventData, IOEventDataType dataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;

        void setOnDataReady(std::function<void(const char*)> callback);
    };

}}

#endif //HOMEGENIE_MINI_RCSWITCHHANDLER_H
