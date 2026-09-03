/*
 * HomeGenie-Mini (c) 2018-2026 G-Labs
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

#include "LiteScriptHandler.h"

extern MatrixConfig ledsConfig;
extern uint8_t currentStyleIndex;

namespace Service { namespace API {

    void LiteScriptHandler::init() {
    }

    bool LiteScriptHandler::canHandleDomain(String *domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
    }

    bool LiteScriptHandler::handleRequest(APIRequest *command, ResponseCallback *responseCallback) {
        if (command->Domain == (IOEventDomains::HomeAutomation_HomeGenie)
            && command->Address == "Automation") {

            if (command->Command == "Script.Run") {
                // Temp local string allocated only during this API handler execution
                String tempScript;
                if (command->Data.length() > 0) {
                    tempScript = command->Data;
                } else {
                    tempScript = Utility::urlDecode(command->OptionsString);
                }

                // Compile directly from the temp string
                compileScript(tempScript, compiledScript, ledsConfig);

                // Clean permanently stored string to keep the heap free of 14+ KB duplication
                liteScript.clear();

                startTime = millis();
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);
                switchToScriptFx();

                return true;
            }

            if (command->Command == "Script.Clear") {
                liteScript.clear();
                compiledScript.clear(); // Ensure the binary buffer is also cleared
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);
                switchToScriptFx();

                return true;
            }

            }
        return false;
    }

    bool LiteScriptHandler::handleEvent(IIOEventSender *, const char *domain, const char *address,
        const char *eventPath, void *eventData, IOEventDataType) {
        return false;
    }

    Module * LiteScriptHandler::getModule(const char *domain, const char *address) {
        return module;
    }

    LinkedList<Module *> * LiteScriptHandler::getModuleList() {
        return &moduleList;
    }

    void LiteScriptHandler::switchToScriptFx() {
        currentStyleIndex = 20; // switch to `SCRIPT` fx to show result
        auto msg = std::make_shared<QueuedMessage>(module, "FX.Style", "script");
        module->setProperty("FX.Style", "script");
        HomeGenie::getInstance()->getEventRouter().signalEvent(msg);
    }
}}
