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

#ifndef HOMEGENIE_MINI_CAMERADISPLAYACTIVITY_H
#define HOMEGENIE_MINI_CAMERADISPLAYACTIVITY_H

#include "HomeGenie.h"

#ifdef ENABLE_UI

#include "data/Module.h"
#include "service/api/APIRequest.h"
#include "ui/Activity.h"

#ifdef ESP_CAMERA_SUPPORTED
#include "../../examples/smart-sensor/io/sensors/Esp32Camera.h"
#endif

#include <HTTPClient.h>

#define OVERLAY_MESSAGE_CONNECTING "Connecting..."

namespace UI { namespace Activities { namespace Monitor {

    using namespace Data;
    using namespace Service;
    using namespace Service::ModuleApi;
    using namespace Service::WidgetApi;

    struct JpegDimensions {
        float width{};
        float height{};
        void reset() {
            width = 0;
            height = 0;
        }
    };

    class CameraDisplayActivity : public Activity, public APIHandler {
    public:
        LinkedList<Module*> moduleList;
        Module module;
        bool isEsp32Camera = false;

        explicit CameraDisplayActivity(const char* moduleAddress);
        ~CameraDisplayActivity();

        void onStart() override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void init() override {}
        bool canHandleDomain(String* domain) override;
        bool handleRequest(APIRequest* request, ResponseCallback* callback) override;
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override;
        Module* getModule(const char* domain, const char* address) override;
        LinkedList<Module*>* getModuleList() override;

        void onTap(PointerEvent e) override;

    private:
        bool isActivityReady = false;
        String jpegFeedUrl;
        String feedUrlConfigKey;
        String feedError;
        bool showOnScreenDisplay = true;
        LGFX_Sprite* view{};
        [[noreturn]] static void * worker(void* activity);

        uint8_t* imageData{};
        unsigned long readyTimestamp{};
        float averageFps{};

        String imageResolution = "5";
        String imageQuality = "10";

        void setJpegFeedUrl(const String& feedUrl);
        uint8_t* getJpegImage(const String& imageUrl);
        static JpegDimensions getJpegDimensions(const uint8_t* jpg_data, size_t data_size);

        // UI options update listener
        class RemoteCameraUrlUpdateListener : public ModuleParameter::UpdateListener {
        public:
            explicit RemoteCameraUrlUpdateListener(CameraDisplayActivity* h) {
                host = h;
            }
            void onUpdate(ModuleParameter* option) override {
                String remoteCameraUrl = option->value + String("/") + String(CameraApi::Camera_GetPicture);
                host->setJpegFeedUrl(remoteCameraUrl);
            }
        private:
            CameraDisplayActivity* host;
        };
        RemoteCameraUrlUpdateListener* optionUpdateListener = nullptr;

        JpegDimensions frameSize;

        HTTPClient http;
        size_t imageLen = 0;
    };

}}}

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_CAMERADISPLAYACTIVITY_H
