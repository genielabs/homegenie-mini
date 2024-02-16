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


#include "Task.h"
#include "TaskManager.h"

#include "io/gpio/GPIOPort.h"
#include "io/IOEventPaths.h"
#include "io/IOManager.h"
#include "net/NetManager.h"
#include "service/api/APIRequest.h"
#include "service/api/APIHandler.h"
#include "service/api/HomeGenieHandler.h"
#include "service/EventRouter.h"
#include "service/Module.h"


#define HOMEGENIEMINI_NS_PREFIX            "Service::HomeGenie"

namespace Service {

    using namespace IO;
    using namespace Net;
    using namespace Service::API;

    class HomeGenie: IIOEventReceiver, NetRequestHandler {
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
        void onIOEvent(IIOEventSender *sender, const char* domain, const char* address, const unsigned char *eventPath, void *eventData, IOEventDataType dataType) override;

        // NetRequestHandler overrides
        bool onNetRequest(void* sender, const char* requestMessage, ResponseCallback* responseCallback) override;

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

        const char* getModuleJSON(Module* module);
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

        // Service Button handling

        volatile int64_t buttonPressStart = 0;
        volatile bool buttonPressed = false;
        static void buttonChange() {
            getInstance()->buttonPressed = (digitalRead(Config::ServiceButtonPin) == LOW);
            if (getInstance()->buttonPressed) {
                getInstance()->buttonPressStart = millis();
            }
        }
        static void checkServiceButton() {
            int64_t elapsed = 0;
            if (getInstance()->buttonPressed) {
                // released
                elapsed = millis() - getInstance()->buttonPressStart;
                if (elapsed > Config::WpsModePushInterval) {
                    noInterrupts();
                    getInstance()->getNetManager().getWiFiManager().configure();
                    interrupts();
                }
            }
        }
        bool statusLedOn = false;
        uint64_t statusLedTs = 0;
        void statusLedLoop() {
            if (WiFi.isConnected()) {
                // when connected the LED will blink quickly every 2 seconds
                if (millis() - statusLedTs > 1950 && !statusLedOn) {
                    statusLedOn = true;
                    digitalWrite(Config::StatusLedPin, HIGH);
                    statusLedTs = millis();
                } else if (statusLedOn && millis() - statusLedTs > 50) {
                    statusLedOn = false;
                    digitalWrite(Config::StatusLedPin, LOW);
                    statusLedTs = millis();
                }
            } else {
                // if not connected the LED will blink quickly every 200ms
                if (millis() - statusLedTs > 100 && !statusLedOn) {
                    statusLedOn = true;
                    digitalWrite(Config::StatusLedPin, HIGH);
                    statusLedTs = millis();
                } else if (statusLedOn && millis() - statusLedTs > 100) {
                    statusLedOn = false;
                    digitalWrite(Config::StatusLedPin, LOW);
                    statusLedTs = millis();
                }
            }
        }

    };

}

#endif //HOMEGENIE_MINI_HOMEGENIE_H
