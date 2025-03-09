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
 */

#ifndef HOMEGENIE_MINI_CAMERA_H
#define HOMEGENIE_MINI_CAMERA_H

#include <HomeGenie.h>

namespace Service { namespace API { namespace devices {

    class FrameBuffer {
    public:
        uint8_t* buffer;
        size_t length;
    };

    class Camera : public APIHandler {
    public:
        Camera(const char* domain, const char* address, const char* name);

        void init() override;

        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest *request, ResponseCallback* responseCallback) override;
        bool handleEvent(IIOEventSender *sender,
                         const char* domain, const char* address,
                         const char *eventPath, void *eventData, IOEventDataType dataType) override;

        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;

        Module* module;

        void onFrameRequest(std::function<FrameBuffer*()> callback) {
            setOnFrameRequestCallback = std::move(callback);
        }
        void onFrameRelease(std::function<void()> callback) {
            setOnFrameReleaseCallback = std::move(callback);
        }

    private:
        LinkedList<Module*> moduleList;
        std::function<FrameBuffer*()> setOnFrameRequestCallback = nullptr;
        std::function<void()> setOnFrameReleaseCallback = nullptr;
    };


}}}

#endif //HOMEGENIE_MINI_CAMERA_H
