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
        if (Config::ServiceButtonPin != -1) {
            pinMode(Config::ServiceButtonPin, INPUT_PULLUP);
//            attachInterrupt(digitalPinToInterrupt(Config::ServiceButtonPin), buttonChange, CHANGE);
        }

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

#ifndef DISABLE_DATA_PROCESSING
        Statistics();
        auto dataProcessingHandler = new DataProcessingHandler();
        addAPIHandler(dataProcessingHandler);
#endif

#ifdef CONFIG_ENABLE_POWER_MANAGER
        PowerManager::setWakeUpInterval(0);
        PowerManager::init();
#endif

        programs.setStatusCallback(this);
        programs.load();
#ifndef DISABLE_MQTT_CLIENT
        // create "MQTT Network" program if not already there
        auto mqttNetwork = programs.getItem(MQTT_NETWORK_CONFIGURATION);
        if (mqttNetwork == nullptr) {
            mqttNetwork = new Program(MQTT_NETWORK_CONFIGURATION, "MQTT Network", "");
            programs.addItem(mqttNetwork);
            programs.save();
        }
        auto programStatus = mqttNetwork->getProperty(IOEventPaths::Program_Status);
        if (programStatus == nullptr) {
            mqttNetwork->properties.add(new ModuleParameter(IOEventPaths::Program_Status, ""));
        }
        homeGenieHandler->addModule(mqttNetwork);
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
        xTaskCreate(
            reinterpret_cast<TaskFunction_t>(Scheduler::worker),
            "Scheduler",
            ESP_TASK_TIMER_STACK,
            nullptr,
            tskIDLE_PRIORITY,
            nullptr
        );
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

#ifndef DISABLE_MQTT_CLIENT
        // Configure MQTT client
        auto mqttProgram = programs.getItem(MQTT_NETWORK_CONFIGURATION);
        if (mqttProgram != nullptr) {
            netManager.getMQTTClient().configure(mqttProgram->properties);
            if (mqttProgram->isEnabled) {
                netManager.getMQTTClient().enable();
            }
        }
#endif

        // Add System Diagnostics event handler
        auto systemDiagnostics = new System::Diagnostics();
        systemDiagnostics->setModule(getDefaultModule());
        ioManager.addEventSender(systemDiagnostics);

        Logger::info("READY.");
    }

    void HomeGenie::loop() {
        Logger::verbose(":%s loop() >> BEGIN", HOMEGENIEMINI_NS_PREFIX);

        statusLedLoop();
        checkServiceButton();

        // HomeGenie-Mini Terminal CLI
        if (Serial.available() > 0) {
            String cmd = Serial.readStringUntil('\n');
            if (cmd.startsWith("#")) {
                Config::handleConfigCommand(cmd);
            } else if (!cmd.isEmpty()) {
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
        if (netManager.getTimeClient()->isTimeSet() || netManager.getTimeClient()->getNTPClient().isUpdated()) {

            ProgramEngine::run(schedule);

        } else {

            Logger::warn(":%s [Scheduler::Event] >> ['%s' skipped because time client is not updated]", HOMEGENIEMINI_NS_PREFIX,
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

        // Search and invoke event handlers for this message
        String d = domain;
        for (int i = 0; i < handlers.size(); i++) {
            auto handler = handlers.get(i);
            if (handler->canHandleDomain(&d) && handler->handleEvent(sender, domain, address, eventPath, eventData, dataType)) {
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
            auto json = HomeGenie::createModuleParameter(param->name.c_str(), param->value.c_str(), param->updateTime.c_str());
            parameters += String(json);
            free((void*)json);
            if (p < module->properties.size() - 1) {
                parameters += ",";
            }
        }
        return HomeGenie::createModule(module->domain.c_str(), module->address.c_str(),
                                       module->name.c_str(), module->description.c_str(), module->type.c_str(),
                                       parameters.c_str());
    }

    unsigned int HomeGenie::writeModuleJSON(ResponseCallback *responseCallback, String* domain, String* address) {
        auto module = getModule(domain, address);
        if (module != nullptr) {
            auto json = getModuleJSON(module);
            responseCallback->write(json);
            free((void*)json);
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
                auto json = getModuleJSON(module);
                out += String(json);
                free((void*)json);
                responseCallback->write(out.c_str());
            }
        }
        out = "]\n";
        responseCallback->write(out.c_str());
        return responseCallback->contentLength;
    }

    unsigned int HomeGenie::writeGroupListJSON(ResponseCallback *responseCallback) {
        bool firstModule = true;
        String defaultGroupName = Config::getSetting(CONFIG_KEY_device_group, "Dashboard");
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


#ifndef DISABLE_DATA_PROCESSING
    unsigned int HomeGenie::writeParameterHistoryJSON(ModuleParameter* parameter, ResponseCallback *outputCallback, int pageNumber, int pageSize, double rangeStart, double rangeEnd, double maxWidth) {
        bool firstItem = true;
        auto count = Statistics::getHistorySize();
        if (pageSize == 0) pageSize = STATS_HISTORY_RESULTS_DEFAULT_PAGE_SIZE;
        int currentPage = 0;
        int i = 0;
        // normalize range start/end
        if (rangeEnd > 0) {
            for (; i < count; i++) {
                auto statValue = Statistics::getHistory(i);
                if (statValue->timestamp <= rangeEnd) {
                    rangeEnd = statValue->timestamp;
                    break;
                }
            }
        }
        if (rangeStart > 0) {
            double start = rangeStart;
            for (; i < count; i++) {
                auto statValue = Statistics::getHistory(i);
                if (statValue->timestamp >= rangeStart) {
                    start = statValue->timestamp;
                } else {
                    break;
                }
            }
            rangeStart = start;
        }

        double step = maxWidth > 0 ? (rangeEnd - rangeStart) / maxWidth : 0;
        String out = R"({"HistoryLimit": )" + String("1440") + R"(, "HistoryLimitSize": 2000, "History": [)";
        outputCallback->write(out.c_str());
        int itemCount = 0;
        for (int i = 0; i < count; i++) {
            auto statValue = Statistics::getHistory(i);

            double timestamp = statValue->timestamp;
            double value = statValue->value;

            if (statValue->parameter != parameter)
                continue;

            if (rangeStart > 0 && timestamp < rangeStart)
                break;
            if (rangeEnd > 0 && timestamp > rangeEnd)
                continue;

            if (step > 0) {
                auto ts = timestamp - fmod(timestamp, step) + step;
                int tot = 1;
                for (int j = i + 1; j < count; j++) {
                    statValue = Statistics::getHistory(j);
                    if (statValue->timestamp < ts - step || statValue->timestamp < rangeStart) break;
                    if (statValue->parameter == parameter) {
                        value += statValue->value;
                        tot++;
                    }
                    i = j;
                }
                // average value in the given timestamp range
                value = (value / tot);
            }

            if (currentPage == pageNumber) {
                if (!firstItem) {
                    out = ",\n";
                } else {
                    firstItem = false;
                    out = "\n";
                }

                int milli = (timestamp * 1000.0f) - (trunc(timestamp) * 1000);
                auto secs = (time_t) timestamp;
                char buf[sizeof "1990-06-11T11:11:00.000Z"];
                strftime(buf, sizeof buf, "%FT%T", gmtime(&secs));
                sprintf(buf, "%s.%03dZ", buf, milli);

                out += R"({"Value": )" + String(value) + R"(, "Timestamp": ")" + String(buf) +
                       R"(", "UnixTimestamp": )" + String(timestamp * 1000, 3) + R"(})";

                outputCallback->write(out.c_str());
            }

            itemCount++;
            if (pageSize != -1 && itemCount == pageSize) {
                if (currentPage == pageNumber) {
                    break;
                }
                currentPage++;
                itemCount = 0;
            }

        }
        out = "\n]}\n";

        outputCallback->write(out.c_str());
        return outputCallback->contentLength;
    }
#endif

    const char* HomeGenie::createModuleParameter(const char *name, const char *value, const char *timestamp) {
        static const char *parameterTemplate = R"({
  "Name": "%s",
  "Value": "%s",
  "Description": "%s",
  "FieldType": "%s",
  "UpdateTime": "%s"
})";
        auto v = String(value);
        Utility::simpleJsonStringEscape(v);
        ssize_t size = snprintf(nullptr, 0, parameterTemplate,
                                name, v.c_str(), "", "", timestamp
        ) + 1;
#ifdef BOARD_HAS_PSRAM
        char *parameterJson = (char *) ps_malloc(size);
#else
        char *parameterJson = (char *) malloc(size);
#endif
        snprintf(parameterJson, size, parameterTemplate,
                 name, v.c_str(), "", "", timestamp
        );
        return parameterJson;
    }

    const char* HomeGenie::createModule(const char *domain, const char *address, const char *name, const char *description,
                                   const char *deviceType, const char *parameters) {
        static const char *moduleTemplate = R"({
  "Name": "%s",
  "Description": "%s",
  "DeviceType": "%s",
  "Domain": "%s",
  "Address": "%s",
  "Properties": [%s]
})";
        auto d = String(description);
        Utility::simpleJsonStringEscape(d);
        ssize_t size = snprintf(nullptr, 0, moduleTemplate,
                                name, d.c_str(), deviceType,
                                domain, address,
                                parameters
        ) + 1;
#ifdef BOARD_HAS_PSRAM
        char *moduleJson = (char *) ps_malloc(size);
#else
        char *moduleJson = (char *) malloc(size);
#endif
        snprintf(moduleJson, size, moduleTemplate,
                 name, d.c_str(), deviceType,
                 domain, address,
                 parameters
        );
        return moduleJson;
    }

}
