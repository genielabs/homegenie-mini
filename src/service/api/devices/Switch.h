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

#ifndef HOMEGENIE_MINI_SWITCH_H
#define HOMEGENIE_MINI_SWITCH_H

#include <HomeGenie.h>

namespace Service { namespace API { namespace devices {

    enum SwitchStatus {
        SWITCH_STATUS_NOT_SET = -1,
        SWITCH_STATUS_OFF,
        SWITCH_STATUS_ON,
    };

    class Switch: public Task, public APIHandler {
    public:
        Switch(const char* domain, const char* address, const char* name);
        void init() override;
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest*, ResponseCallback*) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const unsigned char *eventPath, void* eventData, IOEventDataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;
        void onSetStatus(std::function<void(SwitchStatus)> callback) {
            setStatusCallback = std::move(callback);
        }
    private:
        LinkedList<Module*> moduleList;
        std::function<void(SwitchStatus)> setStatusCallback = nullptr;
    protected:
        Module* module;
        SwitchStatus status = SWITCH_STATUS_NOT_SET;
        float onLevel = 1;
    };

}}}

#endif //HOMEGENIE_MINI_SWITCH_H
