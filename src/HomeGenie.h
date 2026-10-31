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
 * - 2019-01-10 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_HOMEGENIE_H
#define HOMEGENIE_MINI_HOMEGENIE_H


#ifdef CONFIG_ENABLE_POWER_MANAGER
#include "PowerManager.h"
#endif

#include "Task.h"
#include "TaskManager.h"

#ifndef DISABLE_AUTOMATION
#include "automation/ProgramEngine.h"
#include "automation/Scheduler.h"
#endif
#include "data/ProgramStore.h"

#include "data/Module.h"
#include "io/gpio/GPIOPort.h"
#include "io/IOEventPaths.h"
#include "io/IOManager.h"
#include "io/sys/Diagnostics.h"
#include "net/NetManager.h"
#include "service/api/APIRequest.h"
#include "service/api/APIHandler.h"
#ifndef DISABLE_DATA_PROCESSING
#include "service/api/DataProcessingHandler.h"
#endif
#include "service/api/HomeGenieHandler.h"
#include "service/EventRouter.h"


#define HOMEGENIEMINI_NS_PREFIX     "Service::HomeGenie"
#define MQTT_NETWORK_CONFIGURATION  "77"

namespace Service {

    using namespace IO;
    using namespace Data;
#ifndef DISABLE_DATA_PROCESSING
    using namespace Data::Processing;
#endif
    using namespace Net;
    using namespace Service::API;
#ifndef DISABLE_AUTOMATION
    using namespace Automation;
#endif

    class HomeGenie: IIOEventReceiver, NetRequestHandler, ProgramStatusListener
#ifndef DISABLE_AUTOMATION
            , SchedulerListener
#endif
            {
    public:
        static HomeGenie* getInstance() {
            if (serviceInstance == nullptr) {
                serviceInstance = new HomeGenie();
            }
            return serviceInstance;
        };

        HomeGenie();

        void begin();

        // Task overrides
        void loop();

        // IIOEventReceiver overrides
        void onIOEvent(IIOEventSender *sender, const char* domain, const char* address, const char *eventPath, void *eventData, IOEventDataType dataType) override;

        // NetRequestHandler overrides
        bool onNetRequest(void* sender, const char* requestMessage, ResponseCallback* responseCallback) override;

#ifndef DISABLE_AUTOMATION
        // SchedulerListener events
        void onSchedule(Schedule* schedule) override;
#endif

        // ProgramStatusListener events
        void onProgramEnabled(Program* program) override {
#ifndef DISABLE_MQTT_CLIENT
            if (program->address == MQTT_NETWORK_CONFIGURATION) {
                auto mqttNetwork = programs.getItem(MQTT_NETWORK_CONFIGURATION);
                mqttNetwork->setProperty(IOEventPaths::Program_Status, "Running");
                auto m = std::make_shared<QueuedMessage>();
                m->domain = IOEventDomains::HomeAutomation_HomeGenie_Automation;
                m->sender = MQTT_NETWORK_CONFIGURATION;
                m->event = IOEventPaths::Program_Status;
                m->value = "Running";
                eventRouter.signalEvent(m);
                netManager.getMQTTClient().enable();
            }
#endif
            programs.save();
        }
        void onProgramDisabled(Program* program) override {
#ifndef DISABLE_MQTT_CLIENT
            if (program->address == MQTT_NETWORK_CONFIGURATION) {
                auto mqttNetwork = programs.getItem(MQTT_NETWORK_CONFIGURATION);
                mqttNetwork->setProperty(IOEventPaths::Program_Status, "Stopped");
                auto m = std::make_shared<QueuedMessage>();
                m->domain = IOEventDomains::HomeAutomation_HomeGenie_Automation;
                m->sender = MQTT_NETWORK_CONFIGURATION;
                m->event = IOEventPaths::Program_Status;
                m->value = "Stopped";
                eventRouter.signalEvent(m);
                netManager.getMQTTClient().disable();
            }
#endif
            programs.save();
        }

        /**
         *
         * @param handler
         * @return
         */
        bool addAPIHandler(APIHandler* handler);
        /**
         *
         * @param index
         * @param handler
         * @return
         */
        bool addAPIHandler(int index, APIHandler* handler);
        /**
         *
         * @param handler
         * @return
         */
        bool addIOHandler(IIOEventSender* handler);

        /**
         *
         * @param request
         * @param responseCallback
         * @return
         */
        bool api(APIRequest *request, ResponseCallback* responseCallback);


        NetManager& getNetManager();
        IOManager& getIOManager();
        EventRouter& getEventRouter();

        ProgramStore programs;

        Module* getDefaultModule();
        Module* getModule(String* domain, String* address);

        static const char* getModuleJSON(Module* module);
        unsigned int writeModuleListJSON(ResponseCallback *outputCallback);
        unsigned int writeModuleJSON(ResponseCallback *outputCallback, String* domain, String* address);
        unsigned int writeGroupListJSON(ResponseCallback *outputCallback);
#ifndef DISABLE_DATA_PROCESSING
        unsigned int writeParameterHistoryJSON(ModuleParameter* parameter, ResponseCallback *outputCallback, int pageNumber = 0, int pageSize = STATS_HISTORY_RESULTS_DEFAULT_PAGE_SIZE, double rangeStart = 0, double rangeEnd = 0, double maxWidth = 0);
#endif
        static const char* createModule(const char *domain, const char *address, const char *name, const char* description, const char *deviceType, const char *parameters);
        static const char* createModuleParameter(const char *name, const char* value, const char *timestamp);

    private:
        static HomeGenie* serviceInstance;
        NetManager netManager;
        IOManager ioManager;
        EventRouter eventRouter;
        LinkedList<APIHandler*> handlers;

#ifndef DISABLE_AUTOMATION
        Scheduler scheduler;
        ProgramEngine programEngine;
#endif

        // Service Button handling

        DeviceSetup::ServiceButtonHold serviceButton;
        DeviceSetup::StatusIndicator statusIndicator;

        static void checkServiceButton() {
            auto hg = getInstance();
            if (Config::wpsRequest) {
                Config::wpsRequest = false;
                hg->getNetManager().getWiFiManager().configure();
                return;
            }
            bool pressed = Config::ServiceButtonPin >= 0 && digitalRead(Config::ServiceButtonPin) == LOW;
            if (Config::configurationRequested) {
                if (pressed) return;
                Config::configurationRequested = false;
                if (!Config::restartInConfigurationMode()) Logger::error("Could not save configuration request.");
                return;
            }
            if (Config::ServiceButtonPin < 0) return;
            // Act only after a debounced release. A held button cannot trigger
            // both entry and reset, nor survive an internal restart as a new hold.
            if (hg->serviceButton.update(pressed, uint32_t(millis()), Config::ConfigureButtonPushInterval)) {
                if (Config::isTemporaryConfiguration()) {
                    if (!hg->getNetManager().getWiFiManager().configure(true)) {
                        Logger::error("Could not reset credentials and save configuration request.");
                    }
                } else if (!Config::restartInConfigurationMode()) {
                    Logger::error("Could not save configuration request.");
                }
            }
        }

        void statusLedLoop() {
#ifdef ESP32
            // Retain the existing explicit WPS timeout/cancel handling.
            if (Net::WiFiManager::wpsIsTimedOut()) Net::WiFiManager::wpsCancel();
#endif
            DeviceSetup::Mode mode = Config::isFullConfiguration() ? DeviceSetup::Mode::Provisioning
                : Config::isConfigurationMode() ? DeviceSetup::Mode::Temporary : DeviceSetup::Mode::Normal;
            // Preview the action at the hold threshold so the user knows when
            // to release. Execution is still deferred until release.
            if (serviceButton.ready(uint32_t(millis()), Config::ConfigureButtonPushInterval)) {
                mode = DeviceSetup::configurationEntryMode(Config::isTemporaryConfiguration());
            }
            bool configurationActive = mode != DeviceSetup::Mode::Normal;
            bool configurationChanged = configurationActive != Config::statusLedConfigurationActive;
            Config::statusLedConfigurationActive = configurationActive;
            bool on = statusIndicator.update(uint32_t(millis()), WiFi.isConnected(), mode, Config::hasWiFiCredentials());
            if (on != Config::isStatusLedOn || configurationChanged) {
                if (on) Config::statusLedOn(); else Config::statusLedOff();
            }
        }

        static char* allocateJsonBuffer(size_t size) {
            char* buffer = nullptr;
#ifdef ESP32
            if (heap_caps_get_total_size(MALLOC_CAP_SPIRAM) > 0) {
                buffer = (char*) heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
            }
#endif
            if (!buffer) {
                buffer = (char*) malloc(size);
            }
            return buffer;
        }

    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H
