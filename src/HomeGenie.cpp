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

#include "HomeGenie.h"

namespace Service {
    HomeGenie* HomeGenie::serviceInstance = nullptr;

    HomeGenie::HomeGenie() {
        eventRouter.withNetManager(netManager);

        // Load system settings and setup status led
        Config::init();

        // Setup button
        pinMode(Config::ServiceButtonPin, INPUT_PULLUP);
//        attachInterrupt(digitalPinToInterrupt(Config::ServiceButtonPin), buttonChange, CHANGE);

        // Logger initialization
        Logger::begin(LOG_LEVEL_TRACE);

        // Welcome message
        Logger::info("%s %s", CONFIG_DEVICE_MODEL_NAME, CONFIG_DEVICE_MODEL_NUMBER);
        Logger::info("Booting...");

        // Default service handler
        auto gpioPort = new GPIOPort();
        addIOHandler(gpioPort);
        auto homeGenieHandler = new HomeGenieHandler(gpioPort);
        addAPIHandler(homeGenieHandler);
#ifdef CONFIG_ENABLE_POWER_MANAGER
        PowerManager::setWakeUpInterval(0);
        PowerManager::init();
#endif
        Logger::info("+ Starting HomeGenie service");
    }

    void HomeGenie::begin() {
        netManager.begin();
        netManager.setRequestHandler(this);
#ifndef DISABLE_BLUETOOTH_LE
        netManager.getBLEManager().addHandler(this);
#endif
        ioManager.begin();
        ioManager.setEventReceiver(this);

        // Initialize custom API handlers
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            handler->init();
        }

#ifndef DISABLE_AUTOMATION

        // init automation programs engine
        Automation::ProgramEngine::begin([this](void* sender, const char* apiCommand, ResponseCallback* callback){
            this->onNetRequest(sender, apiCommand, callback);
        });

        // set scheduler callback
        Automation::Scheduler::setListener(this);

#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
        /*
        xTaskCreate(
            reinterpret_cast<TaskFunction_t>(Scheduler::loop),
            "Scheduler",
            ESP_TASK_TIMER_STACK,
            nullptr,
            ESP_TASK_TIMER_PRIO - 1,
            nullptr
        );*/
        xTaskCreate(
            reinterpret_cast<TaskFunction_t>(ProgramEngine::worker),
            "ScheduledTask",
            10240, // this might require some adjustments
            nullptr,
            tskIDLE_PRIORITY + 1,
            nullptr
        );
#endif

#endif

        Logger::info("READY.");
    }

    void HomeGenie::loop() {
        Logger::verbose(":%s loop() >> BEGIN", HOMEGENIEMINI_NS_PREFIX);

        statusLedLoop();
        checkServiceButton();

        // HomeGenie-Mini Terminal CLI
        if (Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            if (!cmd.isEmpty()) {
                // TODO: implement SerialCallback
                auto callback = DummyResponseCallback();
                onNetRequest(this, cmd.c_str(), &callback);
            }
        }

        // TODO: sort of system load index could be obtained by measuring time elapsed for `TaskManager::loop()` method
        TaskManager::loop();

#ifdef CONFIG_ENABLE_POWER_MANAGER
        if (Config::isDeviceConfigured() && PowerManager::canEnterSleepMode()) {

            // TODO: should gracefully stop all activities and drivers

            PowerManager::sleep();
        }
#endif

        Logger::verbose(":%s loop() << END", HOMEGENIEMINI_NS_PREFIX);
    }

#ifndef DISABLE_AUTOMATION
    void HomeGenie::onSchedule(Automation::Schedule *schedule) {
        if (netManager.getTimeClient().isUpdated()) {

            ProgramEngine::run(schedule);

        } else {

            Logger::trace(":%s [Scheduler::Event] >> ['%s' skipped because time client is not updated]", HOMEGENIEMINI_NS_PREFIX,
                          schedule->name.c_str());

        }
    }
#endif

    bool HomeGenie::addAPIHandler(APIHandler* handler) {
        handlers.add(handler);
        return true;
    }
    bool HomeGenie::addIOHandler(IO::IIOEventSender *handler) {
        ioManager.addEventSender(handler);
        return true;
    }

    NetManager &HomeGenie::getNetManager() {
        return netManager;
    }

    IOManager &HomeGenie::getIOManager() {
        return ioManager;
    }

    EventRouter &HomeGenie::getEventRouter() {
        return eventRouter;
    }


    // BEGIN IIOEventReceiver interface methods

    void HomeGenie::onIOEvent(IIOEventSender *sender, const char* domain, const char* address, const char *eventPath, void *eventData,
                              IOEventDataType dataType) {
        Logger::trace(":%s [IOManager::IOEvent] >> [domain '%s' address '%s' event '%s']", HOMEGENIEMINI_NS_PREFIX,
                      domain, address, eventPath);
        String d = domain;
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            if (handler->canHandleDomain(&d)) {
                handler->handleEvent(sender, domain, address, eventPath, eventData, dataType);
                break;
            }
        }
    }

    // END IIOEventReceiver


    // BEGIN NetRequestHandler interface methods

    bool HomeGenie::onNetRequest(void *sender, const char* requestMessage, ResponseCallback* responseCallback) {
        auto command = APIRequest::parse(requestMessage);
        return api(&command, responseCallback);
    }

    // END NetRequestHandler


    bool HomeGenie::api(APIRequest *request, ResponseCallback* responseCallback) {
        bool handled = false;
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            if (handler->canHandleDomain(&request->Domain)) {
                handled = handled || handler->handleRequest(request, responseCallback);
            }
            //if (handled) break;
            yield();
        }
        return handled;
    }


    Module* HomeGenie::getDefaultModule() {
        auto domain = String(IOEventDomains::HomeAutomation_HomeGenie);
        auto address = String(CONFIG_BUILTIN_MODULE_ADDRESS);
        return getModule(&domain, &address);
    }

    Module* HomeGenie::getModule(String* domain, String* address) {
        Module* module = nullptr;
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            if (handler->canHandleDomain(domain)) {
                module = handler->getModule(domain->c_str(), address->c_str());
                if (module != nullptr) break;
            }
        }
        return module;
    }

    const char* HomeGenie::getModuleJSON(Module* module) {
        String parameters = "";
        for(int p = 0; p < module->properties.size(); p++) {
            auto param = module->properties.get(p);
            parameters += HomeGenie::createModuleParameter(param->name.c_str(), param->value.c_str(), param->updateTime.c_str());
            if (p < module->properties.size() - 1) {
                parameters += ",";
            }
        }
        String out = HomeGenie::createModule(module->domain.c_str(), module->address.c_str(),
                                             module->name.c_str(), module->description.c_str(), module->type.c_str(),
                                             parameters.c_str());
        return out.c_str();
    }

    unsigned int HomeGenie::writeModuleJSON(ResponseCallback *responseCallback, String* domain, String* address) {
        auto module = getModule(domain, address);
        if (module != nullptr) {
            String out = getModuleJSON(module);
            responseCallback->write(out.c_str());
        }
        return responseCallback->contentLength;
    }

    unsigned int HomeGenie::writeModuleListJSON(ResponseCallback *responseCallback) {
        bool firstModule = true;
        String out = "[";
        responseCallback->write(out.c_str());
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            auto moduleList = handler->getModuleList();

            for(int m = 0; m < moduleList->size(); m++) {
                auto module = moduleList->get(m);
                if (!firstModule) {
                    out = ",";
                } else {
                    firstModule = false;
                    out = "";
                }
                out += getModuleJSON(module);
                responseCallback->write(out.c_str());
            }
        }
        out = "]\n";
        responseCallback->write(out.c_str());
        return responseCallback->contentLength;
    }

    unsigned int HomeGenie::writeGroupListJSON(ResponseCallback *responseCallback) {
        bool firstModule = true;
        String defaultGroupName = "Dashboard";
#ifndef DISABLE_PREFERENCES
        Preferences preferences;
        preferences.begin(CONFIG_SYSTEM_NAME, true);
        String deviceName = preferences.getString(CONFIG_KEY_device_name, "");
        preferences.end();
        if (deviceName.length() > 0) {
            defaultGroupName = deviceName;
        }
#endif
        String out = R"([{"Name": ")" + defaultGroupName + R"(", "Modules": [)";
        responseCallback->write(out.c_str());
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            auto moduleList = handler->getModuleList();

            for(int m = 0; m < moduleList->size(); m++) {
                if (!firstModule) {
                    out = ",";
                } else {
                    firstModule = false;
                    out = "";
                }
                auto module = moduleList->get(m);
                out += R"({"Address": ")" + module->address + R"(", "Domain": ")" + module->domain + R"("})";
                responseCallback->write(out.c_str());
            }

        }
        out = "]}]\n";
        responseCallback->write(out.c_str());
        return responseCallback->contentLength;
    }


    String HomeGenie::createModuleParameter(const char *name, const char *value, const char *timestamp) {
        static const char *parameterTemplate = R"({
    "Name": "%s",
    "Value": "%s",
    "Description": "%s",
    "FieldType": "%s",
    "UpdateTime": "%s"
  })";
        ssize_t size = snprintf(nullptr, 0, parameterTemplate,
                                name, value, "", "", timestamp
        ) + 1;
        char *parameterJson = (char *) malloc(size);
        snprintf(parameterJson, size, parameterTemplate,
                 name, value, "", "", timestamp
        );
        auto p = String(parameterJson);
        free(parameterJson);
        return p;
    }

    String HomeGenie::createModule(const char *domain, const char *address, const char *name, const char *description,
                                   const char *deviceType, const char *parameters) {
        static const char *moduleTemplate = R"({
  "Name": "%s",
  "Description": "%s",
  "DeviceType": "%s",
  "Domain": "%s",
  "Address": "%s",
  "Properties": [%s]
})";
        ssize_t size = snprintf(nullptr, 0, moduleTemplate,
                                name, description, deviceType,
                                domain, address,
                                parameters
        ) + 1;
        char *moduleJson = (char *) malloc(size);
        snprintf(moduleJson, size, moduleTemplate,
                 name, description, deviceType,
                 domain, address,
                 parameters
        );
        auto m = String(moduleJson);
        free(moduleJson);
        return m;
    }

}
