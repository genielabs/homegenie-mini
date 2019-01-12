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

#ifndef HOMEGENIE_MINI_HOMEGENIE_H
#define HOMEGENIE_MINI_HOMEGENIE_H

#include <io/IOManager.h>
#include "HttpServer.h"

#include <detail/RequestHandler.h>

namespace Service {

    using namespace IO;

    class HomeGenie: RequestHandler {
    public:
        HomeGenie();
        void begin();
        void loop();

        bool canHandle(HTTPMethod method, String uri);
        bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri);

        IOManager& getIOManager();
        //void getHttpServer();
    private:
        IOManager *ioManager;
        HttpServer *httpServer;
        void getBytes(const String &rawBytes, uint8_t *data);
    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H