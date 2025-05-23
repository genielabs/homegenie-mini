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
        float width = 0;
        float height = 0;
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

        explicit CameraDisplayActivity(const char* moduleAddress) {
            setDrawInterval(50); // Task.h
            module.domain  = IO::IOEventDomains::HomeAutomation_HomeGenie;
            module.address = moduleAddress;
            module.type    = ModuleType::Sensor;
            // load stored name for this module
            auto key = String(CONFIG_KEY_ACTIVITY_TITLE); key.concat(moduleAddress);
            module.name = Config::getSetting(key.c_str(), moduleAddress);
            module.setProperty(Implements::Scheduling, "true");
            module.setProperty(Implements::Scheduling_ModuleEvents, "true");
            module.onNameUpdated = [this](const char* oldName, const char* newName) {
                // TODO: show name on the UI
                //if (labelTitle) {
                //    lv_label_set_text(labelTitle, newName);
                //}
                auto key = String(CONFIG_KEY_ACTIVITY_TITLE);
                key.concat(module.address);
                Config::saveSetting(key.c_str(), newName);
            };
            moduleList.add(&module);
            HomeGenie::getInstance()->addAPIHandler(this);
            // Add UI control to set the camera module associated
            // to this CameraDisplayActivity
            optionUpdateListener = new RemoteCameraUrlUpdateListener(this);
            feedUrlConfigKey = String("rcam-"); feedUrlConfigKey.concat(moduleAddress);
            module.addWidgetOption(
                    // name, value
                    CameraApi::Property::RemoteCamera_EndPoint, "",
                    // type
                    UI_WIDGETS_FIELD_TYPE_MODULE_TEXT
                    // options
                    ":any"
                    ":sensor"
                    ":Widget.DisplayModule=homegenie/generic/camerainput"
                    ":uri"
            )->withConfigKey(feedUrlConfigKey.c_str())->addUpdateListener(optionUpdateListener);
            jpegFeedUrl = module.getProperty(CameraApi::Property::RemoteCamera_EndPoint)->value;
        }
        ~CameraDisplayActivity() {
            module.getProperty(CameraApi::Property::RemoteCamera_EndPoint)->removeUpdateListener(optionUpdateListener);
            delete optionUpdateListener;
            // TODO: IMPLEMENT FreeRTOS Task shutdown/delete
        }

        void attach(LGFX_Device* displayDevice) override;
        void onResume() override;
        void onPause() override;
        void onDraw() override;

        void init() override {
        }
        bool canHandleDomain(String* domain) override {
            return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie);
        }
        bool handleRequest(APIRequest* request, ResponseCallback* callback) override {
            return false;
        }
        bool handleEvent(IIOEventSender*,
                         const char* domain, const char* address,
                         const char *eventPath, void* eventData, IOEventDataType) override {
            return false;
        }
        Module* getModule(const char* domain, const char* address) override {
            if (module.domain.equals(domain) && module.address.equals(address))
                return &module;
            return nullptr;
        }
        LinkedList<Module*>* getModuleList() override {
            return &moduleList;
        }

        void onTap(PointerEvent e) override {
            showOnScreenDisplay = !showOnScreenDisplay;
            if (isActivityReady) {
                canvas->clear();
            }
        }

        void setJpegFeedUrl(const String& feedUrl) {
            jpegFeedUrl = feedUrl;
            // override config setting with transformed value (with 'GetPicture' API call appended)
            Config::saveSetting(feedUrlConfigKey.c_str(), jpegFeedUrl);
            IO::Logger::info("Remote camera module set to: %s", jpegFeedUrl.c_str());
        }

        uint8_t* feedJpegImage(const char* imageUrl);
        static JpegDimensions getJpegDimensions(const uint8_t* jpg_data, size_t data_size);

    private:
        bool isActivityReady = false;
        unsigned long readyTimestamp;
        String jpegFeedUrl;
        String feedUrlConfigKey;
        String feedError;
        bool showOnScreenDisplay = true;
        LGFX_Sprite* view = nullptr;
        [[noreturn]] static void * worker(void* activity);

        String imageResolution = "1";
        String imageQuality = "10";

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
