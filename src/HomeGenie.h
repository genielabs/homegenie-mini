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

#include "data/Module.h"
#include "io/gpio/GPIOPort.h"
#include "io/IOEventPaths.h"
#include "io/IOManager.h"
#include "io/sys/Diagnostics.h"
#include "net/NetManager.h"
#include "service/api/APIRequest.h"
#include "service/api/APIHandler.h"
#include "service/api/HomeGenieHandler.h"
#include "service/EventRouter.h"


#define HOMEGENIEMINI_NS_PREFIX            "Service::HomeGenie"

namespace Service {

    using namespace IO;
    using namespace Data;
    using namespace Net;
    using namespace Service::API;
#ifndef DISABLE_AUTOMATION
    using namespace Automation;
#endif

    class HomeGenie: IIOEventReceiver, NetRequestHandler
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

        /**
         *
         * @param handler
         * @return
         */
        bool addAPIHandler(APIHandler* handler);
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

        Module* getDefaultModule();
        Module* getModule(String* domain, String* address);

        static const char* getModuleJSON(Module* module);
        unsigned int writeModuleListJSON(ResponseCallback *outputCallback);
        unsigned int writeModuleJSON(ResponseCallback *outputCallback, String* domain, String* address);
        unsigned int writeGroupListJSON(ResponseCallback *outputCallback);

        static String createModule(const char *domain, const char *address, const char *name, const char* description, const char *deviceType, const char *parameters);
        static String createModuleParameter(const char *name, const char* value, const char *timestamp);

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

        volatile int64_t buttonPressStart = 0;
        volatile bool buttonPressed = false;
        static void buttonChange() {
            auto hg = getInstance();
            bool wasPressed = hg->buttonPressed;
            hg->buttonPressed = (digitalRead(Config::ServiceButtonPin) == LOW);
            if (!wasPressed && hg->buttonPressed) {
                hg->buttonPressStart = millis();
            }
        }
        static void checkServiceButton() {
            if (Config::ServiceButtonPin < 0) {
                return;
            }
            buttonChange();
            int64_t elapsed = 0;
            auto hg = getInstance();
            if (hg->buttonPressed) {
                // released
                elapsed = millis() - hg->buttonPressStart;
                if (elapsed > Config::ConfigureButtonPushInterval) {
                    hg->getNetManager().getWiFiManager().configure();
                }
            }
        }

        uint64_t statusLedTs;
        void statusLedLoop() {
#ifdef ESP32
            if (!Net::WiFiManager::wpsIsRegistering()) {
#endif
                if (WiFi.isConnected()) {
                    // when connected the LED will blink quickly every 2 seconds
                    if (millis() - statusLedTs > 1950 && !Config::isStatusLedOn) {
                        Config::statusLedOn();
                        statusLedTs = millis();
                    } else if (Config::isStatusLedOn && millis() - statusLedTs > 50) {
                        Config::statusLedOff();
                        statusLedTs = millis();
                    }
                } else {
                    // if not connected the LED will blink quickly every 200ms
                    if (millis() - statusLedTs > 100 && !Config::isStatusLedOn) {
                        Config::statusLedOn();
                        statusLedTs = millis();
                    } else if (Config::isStatusLedOn && millis() - statusLedTs > 100) {
                        Config::statusLedOff();
                        statusLedTs = millis();
                    }
                }
#ifdef ESP32
            } else if (Net::WiFiManager::wpsIsTimedOut()) {
                Net::WiFiManager::wpsCancel();
            }
#endif
        }

    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H
