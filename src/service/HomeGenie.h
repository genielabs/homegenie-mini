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
 * - 2019-01-10 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_HOMEGENIE_H
#define HOMEGENIE_MINI_HOMEGENIE_H

#include <LinkedList.h>

#include <Task.h>
#include <io/IOManager.h>
#include <io/IOEventPaths.h>
#include <net/NetManager.h>
#include "EventRouter.h"

#define HOMEGENIEMINI_NS_PREFIX            "Service::HomeGenie"

namespace Service {

    using namespace IO;
    using namespace Net;
    using namespace Service::API;

    class HomeGenie: Task, RequestHandler, IIOEventReceiver {
    public:
        HomeGenie();
        void begin();

        // Task overrides
        void loop();

        // IIOEventSender
        void onIOEvent(IIOEventSender *sender, const unsigned char *eventPath, void *eventData, IOEventDataType dataType);

        // RequestHandler overrides
        bool canHandle(HTTPMethod method, String uri);
        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);

        bool api(APIRequest *request, ESP8266WebServer &server);

        NetManager& getNetManager();
        IOManager& getIOManager();
        EventRouter& getEventRouter();

        String getBuiltinModuleJSON();
        int writeModuleJSON(ESP8266WebServer *server,String &domain, String &address);
        int writeModuleListJSON(ESP8266WebServer *server);
        int writeGroupListJSON(ESP8266WebServer *server);

        static String createModule(const char *domain, const char *address, const char *name, const char* description, const char *deviceType, const char *parameters);
        static String createModuleParameter(const char *name, const char* value, const char *timestamp);
    private:
        NetManager netManager;
        IOManager ioManager;
        EventRouter eventRouter;
    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H
