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
 * - 2019-01-28 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_APIHANDLER_H
#define HOMEGENIE_MINI_APIHANDLER_H

#include "io/IOEvent.h"
#include "service/Module.h"

#include "APIRequest.h"

namespace Service { namespace API {

    using namespace IO;
    using namespace Service;

    class OutputStreamCallback {
    public:
        virtual void write(String &s) = 0;
    };

    class APIHandler {
    public:
        virtual void init() = 0;
        virtual bool canHandleDomain(String* domain) = 0;
        virtual bool handleRequest(APIRequest *request, WebServer &server) = 0;
        virtual bool handleEvent(IIOEventSender *sender, const char* domain, const char* address, const unsigned char *eventPath, void *eventData, IOEventDataType dataType) = 0;
        virtual Module* getModule(const char* domain, const char* address) = 0;
        virtual LinkedList<Module*>* getModuleList() = 0;
    };

    class APIHandlerOutputCallback : public OutputStreamCallback {
        WebServer *server;
    public:
        unsigned int outputLength = 0;
        APIHandlerOutputCallback(WebServer *server) {
            this->server = server;
        }
        void write(String &s) {
            outputLength += s.length();
            if (server != nullptr) {
                server->sendContent(s);
            }
        }
    };

}}

#endif //HOMEGENIE_MINI_APIHANDLER_H
