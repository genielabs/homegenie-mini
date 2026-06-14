//
// Created by gene on 06/06/26.
//

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

                currentStyleIndex = 20; // switch to `SCRIPT` fx to show result

                return true;
            }

            if (command->Command == "Script.Clear") {
                liteScript.clear();
                compiledScript.clear(); // Ensure the binary buffer is also cleared
                responseCallback->writeAll(ApiHandlerResponseStatus::OK);

                currentStyleIndex = 20; // switch to `SCRIPT` fx to show result

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
        return nullptr;
    }

    LinkedList<Module *> * LiteScriptHandler::getModuleList() {
        return &moduleList;
    }
}}
