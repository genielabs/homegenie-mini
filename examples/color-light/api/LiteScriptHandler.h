//
// Created by gene on 06/06/26.
//

#ifndef HOMEGENIE_MINI_LITESCRIPTHANDLER_H
#define HOMEGENIE_MINI_LITESCRIPTHANDLER_H

#include "service/api/APIHandler.h"

#include "../fx/fx_script_utils.h"

namespace Service { namespace API {

    class LiteScriptHandler : public APIHandler {
    private:
        LinkedList<Module*> moduleList;
    public:
        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest*, ResponseCallback*) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;

        String liteScript;
        std::vector<uint8_t> compiledScript;
        unsigned long startTime;
    };

}}

#endif //HOMEGENIE_MINI_LITESCRIPTHANDLER_H
