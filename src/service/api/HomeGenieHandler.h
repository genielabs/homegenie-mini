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

#ifndef HOMEGENIE_MINI_HOMEGENIEHANDLER_H
#define HOMEGENIE_MINI_HOMEGENIEHANDLER_H

#include "HomeGenie.h"

namespace Service { namespace API {

    using namespace IO::GPIO;

    namespace HomeGenieHandlerResponseStatus {
        static const char* ERROR_NO_SCHEDULE_WITH_THE_GIVEN_NAME = R"({ "ResponseStatus": "ERROR", "ResponseMessage": "No schedule with the given name" })";
    }

    class HomeGenieHandler : public APIHandler {
    private:
        GPIOPort* gpioPort;
        LinkedList<Module*> moduleList;
    public:
        explicit HomeGenieHandler(GPIOPort* gpioPort);
        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest *request, ResponseCallback* responseCallback) override;
        bool handleEvent(IIOEventSender *sender, const char* domain, const char* address, const char *eventPath, void *eventData,
                    IOEventDataType dataType) override;
        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;
    };

    static const char* SCHEDULER_ACTION_TEMPLATES PROGMEM = "[\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_turn_on\",\n"
                                                   "    \"script\": \"$$.boundModules.on();\\n\"\n"
                                                   "  },\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_turn_off\",\n"
                                                   "    \"script\": \"$$.boundModules.off();\\n\"\n"
                                                   "  },\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_toggle\",\n"
                                                   "    \"script\": \"$$.boundModules.toggle();\\n\"\n"
                                                   "  },\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_set_level\",\n"
                                                   "    \"script\": \"$$.boundModules.level = $level$;\\n\",\n"
                                                   "    \"config\": {\n"
                                                   "      \"level\": {\n"
                                                   "        \"type\": \"slider\",\n"
                                                   "        \"options\": {\n"
                                                   "          \"min\": 0,\n"
                                                   "          \"max\": 100,\n"
                                                   "          \"step\": 1\n"
                                                   "        },\n"
                                                   "        \"value\": 0\n"
                                                   "      }\n"
                                                   "    }\n"
                                                   "  },\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_set_color\",\n"
                                                   "    \"script\": \"$$.boundModules.colorHsb = '$color$';\\n\",\n"
                                                   "    \"config\": {\n"
                                                   "      \"color\": {\n"
                                                   "        \"type\": \"color\",\n"
                                                   "        \"value\": \"#ff0000\",\n"
                                                   "        \"transform\": \"rgb:hsb\"\n"
                                                   "      }\n"
                                                   "    }\n"
                                                   "  },\n  {\n"
                                                   "    \"id\": \"command_remote_control\",\n"
                                                   "    \"script\": \"cmd = $data$;\\n  for (i = 0; i < cmd.length; i++) {\\n    $$.boundModules\\n      .command('Control.SendRaw')\\n      .submit(cmd[i]);\\n  }\\n\",\n"
                                                   "    \"config\": {\n"
                                                   "      \"data\": {\n"
                                                   "        \"type\": \"capture:Receiver.RawData\",\n"
                                                   "        \"value\": \"\",\n"
                                                   "        \"transform\": \"multiline:array\"\n"
                                                   "      }\n"
                                                   "    }\n"
                                                   "  },\n"
                                                   "  {\n"
                                                   "    \"id\": \"command_thermostat_mode\",\n"
                                                   "    \"script\": \"$$.boundModules\\n  .command('Thermostat.ModeSet')\\n  .submit('$mode$');\\n\",\n"
                                                   "    \"config\": {\n"
                                                   "      \"mode\": {\n"
                                                   "        \"type\": \"select\",\n"
                                                   "        \"options\": [\n"
                                                   "          {\n"
                                                   "            \"id\": \"off\",\n"
                                                   "            \"value\": \"Off\"\n"
                                                   "          },\n"
                                                   "          {\n"
                                                   "            \"id\": \"heat\",\n"
                                                   "            \"value\": \"Heat\"\n"
                                                   "          },\n"
                                                   "          {\n"
                                                   "            \"id\": \"heat_eco\",\n"
                                                   "            \"value\": \"HeatEconomy\"\n"
                                                   "          },\n"
                                                   "          {\n"
                                                   "            \"id\": \"cool\",\n"
                                                   "            \"value\": \"Cool\"\n"
                                                   "          },\n"
                                                   "          {\n"
                                                   "            \"id\": \"cool_eco\",\n"
                                                   "            \"value\": \"CoolEconomy\"\n"
                                                   "          }\n"
                                                   "        ],\n"
                                                   "        \"value\": \"Off\"\n"
                                                   "      }\n"
                                                   "    }\n"
                                                   "  }\n"
                                                   "]";

}}

#endif //HOMEGENIE_MINI_HOMEGENIEHANDLER_H
