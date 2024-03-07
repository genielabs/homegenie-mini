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

#ifndef HOMEGENIE_MINI_TRANSCEIVERHANDLER_H
#define HOMEGENIE_MINI_TRANSCEIVERHANDLER_H

#include <HomeGenie.h>

#include "../configuration.h"
#include "../io/IRReceiver.h"
#include "../io/IRTransmitter.h"

namespace Service { namespace API {

    using namespace IO::IR;

    class IRTransceiverHandler : public APIHandler, IRTransmitterListener {
    private:
        LinkedList<Module*> moduleList;
        IRTransmitter* transmitter;
        IRReceiver* receiver;
    public:
        IRTransceiverHandler(IRTransmitter*,IRReceiver*);
        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest*, ResponseCallback*) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const unsigned char *eventPath, void* eventData, IOEventDataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;
        // IRTransmitterListener
        void onSendStart() override;
        void onSendComplete() override;
    };

}}

#endif //HOMEGENIE_MINI_TRANSCEIVER_H
