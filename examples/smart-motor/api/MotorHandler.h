/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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
 * - 2024-01-06 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_MOTORHANDLER_H
#define HOMEGENIE_MINI_MOTORHANDLER_H

#include <HomeGenie.h>

#include "MotorApi.h"
#include "../io/MotorControl.h"

namespace Service { namespace API {

        using namespace IO::Components;
        using namespace MotorApi;
        using namespace Service::API::ModuleApi;

        class ParameterListener: public ModuleParameter::UpdateListener {
        private:
            MotorControl* motorControl;
        public:
            explicit ParameterListener(MotorControl* sc) {
                motorControl = sc;
            }
            void onUpdate(ModuleParameter* option) override {
                // The '=' symbol is used here as a special
                // character to separate optional value's
                // language-id string from actual value
                auto v = option->value;
                if (v.indexOf("=") > 0) {
                    v = option->value = v.substring(v.indexOf("=") + 1);
                }
                if (!option->getConfigKey().isEmpty()) {
                    motorControl->configure(option->getConfigKey().c_str(), v.c_str());
                }
            }
        };

        class MotorModule: public Module {
        public:
            MotorControl* motorControl;
        };

        class MotorHandler : public APIHandler {
        private:
            LinkedList<MotorModule*> moduleList;
            MotorModule* addModule(int index);

        public:
            explicit MotorHandler();

            void init() override;

            bool canHandleDomain(String* domain) override;
            bool handleRequest(APIRequest *request, ResponseCallback* responseCallback) override;
            bool handleEvent(IIOEventSender *sender,
                             const char* domain, const char* address,
                             const char *eventPath, void *eventData, IOEventDataType dataType) override;

            Module* getModule(const char* domain, const char* address) override;
            LinkedList<Module*>* getModuleList() override;
        };

}}
#endif //HOMEGENIE_MINI_MOTORHANDLER_H
