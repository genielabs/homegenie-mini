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
 * - 2019-01-28 Initial release
 *
 */

#include "HomeGenieHandler.h"

namespace Service { namespace API {

    HomeGenieHandler::HomeGenieHandler(GPIOPort* gpioPort) {
        this->gpioPort = gpioPort;
        // HomeGenie Mini module (default module)
        miniModule = new Module();
        miniModule->domain = IO::IOEventDomains::HomeAutomation_HomeGenie;
        miniModule->address = CONFIG_BUILTIN_MODULE_ADDRESS;
        miniModule->type = ModuleApi::ModuleType::Sensor;
        miniModule->name = Config::system.friendlyName;
        miniModule->description = "HomeGenie Mini node";
        moduleList.add(miniModule);
    }

    void HomeGenieHandler::init() {
        // Add builtin GPIO modules
        for (int m = 0; m < 8; m++) {
            auto in = String("io-typ0");
            in.concat(m + 1);
            auto moduleType = Config::getSetting(in.c_str(), ModuleApi::ModuleType::Dimmer);
            in.replace("-typ0", "-pin0");
            int gpioNum = Config::getSetting(in.c_str(), "-1").toInt();
            if (gpioNum >= 0 && moduleType != ModuleApi::ModuleType::Sensor) { // TODO: enable INPUT channels (Sensor)
                auto address = String(gpioNum);
                auto module = new Module();
                module->domain = IO::IOEventDomains::HomeAutomation_HomeGenie;
                module->address = address;
                module->type = moduleType;
                module->name = "GPIO " + address;
                module->description = "";
                auto propLevel = new ModuleParameter(IOEventPaths::Status_Level, "");
                module->properties.add(propLevel);
                if (moduleType == ModuleApi::ModuleType::Sensor) {
// TODO: ............
                } else {
                    // Recall last stored level (switchable modules only)
                    float level = GPIOPort::loadLevel(gpioNum);
                    level > 0 ? gpioPort->on(gpioNum) : gpioPort->off(gpioNum);
                    propLevel->value = String(level);
                }
                moduleList.add(module);
            }
        }

        updateSystemModule();
    }

    bool HomeGenieHandler::canHandleDomain(String* domain) {
        return domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie)
                ||domain->equals(IO::IOEventDomains::HomeAutomation_HomeGenie_Automation);
    }

    bool HomeGenieHandler::handleRequest(Service::APIRequest *request, ResponseCallback* responseCallback) {
        auto homeGenie = HomeGenie::getInstance();
        if (request->Address == "Automation") {
#ifndef DISABLE_AUTOMATION
            if (request->Command == AutomationApi::Scheduling_Add || request->Command == AutomationApi::Scheduling_Update) {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, request->Data);

                // TODO: verify if cronExpression is parsed correctly

                if (!error.code()) {
                    if (doc.containsKey(ScheduleField::name)
                        && doc.containsKey(ScheduleField::description)
                        && doc.containsKey(ScheduleField::data)
                        && doc.containsKey(ScheduleField::cronExpression)
                        && doc.containsKey(ScheduleField::script)) {

                        // TODO: return a bool value for success/error
                        Scheduler::addSchedule(Scheduler::getScheduleFromJson(doc));

                        responseCallback->writeAll(ApiHandlerResponseText::OK);
                        return true;
                    }
                }

                return false;
            } else if (request->Command == AutomationApi::Scheduling_Get) {
                auto schedule = Scheduler::get(request->getOption(0).c_str());
                if (schedule != nullptr) {
                    JsonDocument doc;
                    auto obj = doc.add<JsonObject>();
                    Scheduler::getJson(&obj, schedule);
                    String output;
                    serializeJson(obj, output);

                    responseCallback->writeAll(output.c_str());
                    return true;
                } else {
                    responseCallback->writeAll(HomeGenieHandlerResponseStatus::ERROR_NO_SCHEDULE_WITH_THE_GIVEN_NAME);
                    return true;
                }
            } else if (request->Command == AutomationApi::Scheduling_ModuleUpdate) {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    JsonDocument doc;
                    deserializeJson(doc, request->Data);
                    auto include = doc["include"].as<JsonArray>();
                    for (auto s: include) {
                        auto scheduleName = s["Value"].as<const char*>();
                        auto schedule = Scheduler::get(scheduleName);
                        if (schedule != nullptr) {
                            bool exits = false;
                            auto boundModules = &schedule->boundModules;
                            for (int i = 0; i < boundModules->size(); i++) {
                                auto bm = boundModules->get(i);
                                auto sid = bm->serviceId;
                                if ((sid.isEmpty() || sid == Config::system.id) && bm->domain == domain && bm->address == address) {
                                    exits = true;
                                    break;
                                }
                            }
                            if (!exits) {
                                auto mr = module->getReference();
                                boundModules->add(mr);
                                Scheduler::save();
                            }
                        } else {
                            responseCallback->writeAll(HomeGenieHandlerResponseStatus::ERROR_NO_SCHEDULE_WITH_THE_GIVEN_NAME);
                            return true;
                        }
                    }
                    auto exclude = doc["exclude"].as<JsonArray>();
                    for (auto s: exclude) {
                        auto scheduleName = s["Value"].as<const char*>();
                        auto schedule = Scheduler::get(scheduleName);
                        if (schedule != nullptr) {
                            auto boundModules = &schedule->boundModules;
                            for (int i = 0; i < boundModules->size(); i++) {
                                auto bm = boundModules->get(i);
                                auto sid = bm->serviceId;
                                if ((sid.isEmpty() || sid == Config::system.id) && bm->domain == domain && bm->address == address) {
                                    auto mr = boundModules->remove(i);
                                    delete mr;
                                    Scheduler::save();
                                    break;
                                }
                            }
                        } else {
                            responseCallback->writeAll(HomeGenieHandlerResponseStatus::ERROR_NO_SCHEDULE_WITH_THE_GIVEN_NAME);
                            return true;
                        }
                    }

                    responseCallback->writeAll(ApiHandlerResponseText::OK);
                    return true;
                } else {
                    // TODO: report error (module not found)
                }
                return false;
            } else if (request->Command == AutomationApi::Scheduling_ListOccurrences) {
                int hours = request->getOption(0).toInt();
                auto dateStart = request->getOption(1); //YYYY-MM-DD HH:mm:ss
                auto cronExpression = request->getOption(2);

                int year, month, day, hour, min, sec;
                if (sscanf(dateStart.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec) == 6)
                {
                    struct tm t;
                    time_t start, end;

                    t.tm_year = year - 1900; // Year - 1900
                    t.tm_mon = month - 1;    // Month, where 0 = jan
                    t.tm_mday = day;         // Day of the month
                    t.tm_hour = hour;
                    t.tm_min = min;
                    t.tm_sec = sec;
                    t.tm_isdst = -1;         // Is DST on? 1 = yes, 0 = no, -1 = unknown

                    start = mktime(&t);
                    end = start + (hours * 60 * 60) - 60;

                    auto list = ExtendedCron::getScheduling(start, end, cronExpression);
                    /*
                    String s;
                    s += (R"([{ "CronExpression": ")");
                    s += (cronExpression.c_str());
                    s += (R"(", "StartDate": ")");
                    s += (dateStart.c_str());
                    s += (R"(", "Occurrences": [)");

                    for (int i = 0; i < list.size(); i++) {
                        auto o = list.get(i);
                        String sv = String(o) + String("000");
                        s += (sv.c_str());
                        if (i < list.size() - 1) {
                            s += (",");
                        }
                    }
                    s += ("]}]");
                    responseCallback->writeAll(s.c_str());
                    //*/
                    //*
                    JsonDocument doc;
                    auto arr = doc.add<JsonArray>();
                    auto obj = arr.add<JsonObject>();
                    obj["CronExpression"] = cronExpression;
                    obj["StartDate"] = dateStart;
                    auto occurrencesList = obj["Occurrences"] = doc.add<JsonArray>();
                    for (time_t o : list) {
                        occurrencesList.add((long long)o * 1000); // convert to milliseconds
                    }
                    String output;
                    serializeJson(arr, output);

                    responseCallback->writeAll(output.c_str());
                    //*/
                    return true;
                } else {
                    // TODO: report date format error?
                }

                return false;
            } else if (request->Command == AutomationApi::Scheduling_Enable) {
                Scheduler::enableSchedule(request->getOption(0).c_str());
                responseCallback->writeAll(ApiHandlerResponseText::OK);
                return true;
            } else if (request->Command == AutomationApi::Scheduling_Disable) {
                Scheduler::disableSchedule(request->getOption(0).c_str());
                responseCallback->writeAll(ApiHandlerResponseText::OK);
                return true;
            } else if (request->Command == AutomationApi::Scheduling_Delete) {
                // TODO: return -1 or the index of deleted item
                Scheduler::deleteSchedule(request->getOption(0).c_str());
                responseCallback->writeAll(ApiHandlerResponseText::OK);
                return true;
            } else if (request->Command == AutomationApi::Scheduling_List) {
                responseCallback->writeAll(Scheduler::getJsonList().c_str());
                return true;
            } else if (request->Command == AutomationApi::Scheduling_Templates) {
                responseCallback->writeAll(SCHEDULER_ACTION_TEMPLATES);
                return true;
            } else if (request->Command == AutomationApi::Programs_Enable || request->Command == AutomationApi::Programs_Disable) {
                auto address = request->OptionsString.c_str();
                auto program = homeGenie->programs.getItem(address);
                if (program != nullptr) {
                    if (request->Command == AutomationApi::Programs_Enable) {
                        homeGenie->programs.enable(program);
                    } else {
                        homeGenie->programs.disable(program);
                    }
                    homeGenie->programs.save();
                    responseCallback->writeAll(ApiHandlerResponseText::OK);
                    return true;
                }
            }
#endif
        } else if (request->Address == "Config") {
            if (request->Command == ConfigApi::Groups_List) {
                responseCallback->beginGetLength();
                homeGenie->writeGroupListJSON(responseCallback);
                responseCallback->endGetLength();
                homeGenie->writeGroupListJSON(responseCallback);
                return true;
            } else if (request->Command == ConfigApi::Modules_List) {
                updateSystemModule();
                responseCallback->beginGetLength();
                homeGenie->writeModuleListJSON(responseCallback);
                responseCallback->endGetLength();
                homeGenie->writeModuleListJSON(responseCallback);
                return true;
            } else if (request->Command == ConfigApi::Modules_Get) {
                String domain = request->getOption(0);
                String address = request->getOption(1);
                if (domain.equals(miniModule->domain) && address.equals(miniModule->address)) {
                    updateSystemModule();
                }
                responseCallback->beginGetLength();
                auto contentLength = (size_t)homeGenie->writeModuleJSON(responseCallback, &domain, &address);
                responseCallback->endGetLength();
                if (contentLength == 0) return false;
                homeGenie->writeModuleJSON(responseCallback, &domain, &address);
                return true;
            } else if (request->Command == ConfigApi::Modules_UpdateInfo) {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, request->Data);
                    if (!error.code()) {
                        if (doc.containsKey("name") && doc.containsKey("description")) {
                            module->name = doc["name"].as<String>();
                            module->description = doc["description"].as<String>();
                            // TODO: deviceType ....
// TODO: implement data persistence -- should persist name change
                        }
                    }
                }
            } else if (request->Command == ConfigApi::Modules_ParameterGet) {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    auto parameter = module->getProperty(request->getOption(2));
                    if (parameter != nullptr) {
                        /*
                        {
                          "Name": "ConfigureOptions.Username",
                          "Value": "",
//TODO:                   "Description": "5. Username (optional)",
//TODO:                   "FieldType": "text",
//                        "UpdateTime": "2024-12-08T02:46:23.1173437Z",
//TODO: (?)               "NeedsUpdate": false
                        }
                        */
                        auto jsonParameter = HomeGenie::createModuleParameter(parameter->name.c_str(), parameter->value.c_str(), parameter->updateTime.c_str());
                        responseCallback->writeAll(jsonParameter);
                        free((void*)jsonParameter);
                        return true;
                    }
                }
            } else if (request->Command == ConfigApi::Modules_ParameterSet) {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto module = homeGenie->getModule(&domain, &address);

                bool isProgramConfiguration = domain.equals(IOEventDomains::HomeAutomation_HomeGenie_Automation);
                if (module != nullptr || isProgramConfiguration) {
                    // Collect parameters
                    auto parameters = LinkedList<ModuleParameter>();
                    if (request->Data.length() > 0) {
                        JsonDocument doc;
                        DeserializationError error = deserializeJson(doc, request->Data);
                        if (!error.code()) {
                            auto list = doc.as<JsonObject>();
                            for (JsonPair kv : list) {
                                auto p = ModuleParameter(kv.key().c_str(), kv.value().as<String>());
                                parameters.add(p);
                            }
                        }
                    } else {
                        auto propName = request->getOption(2);
                        auto propValue = WebServer::urlDecode(request->getOption(3));
                        if (!propName.isEmpty()) {
                            auto p = ModuleParameter(propName, propValue);
                            parameters.add(p);
                        }
                    }
                    if (parameters.size() == 0) {
                        return false;
                    }
                    // Update module parameters
                    if (module != nullptr) {
                        for (ModuleParameter p: parameters) {
                            if (p.name.isEmpty()) continue;
                            module->setProperty(p.name, p.value, nullptr, IOEventDataType::Undefined);
                            QueuedMessage m;
                            m.domain = domain;
                            m.sender = address;
                            m.event = p.name;
                            m.value = p.value;
                            homeGenie->getEventRouter().signalEvent(m);
                        }
                        if (!isProgramConfiguration) {
                            responseCallback->writeAll(ApiHandlerResponseText::OK);
                            return true;
                        }
                    }
                    // Update program configuration
                    if (isProgramConfiguration) {
// TODO: move this to a "ProgramManager" class (extend actual ProgramStore)
#ifndef DISABLE_MQTT_CLIENT
                        if (address.equals(MQTT_NETWORK_CONFIGURATION)) {
                            auto mqttNetwork = homeGenie->programs.getItem(MQTT_NETWORK_CONFIGURATION);
                            homeGenie->programs.save();
                            homeGenie->getNetManager().getMQTTClient().configure(mqttNetwork->properties);
                        }
#endif
                        responseCallback->writeAll(ApiHandlerResponseText::OK);
                        return true;
                    }
                    return false;
                }
#ifndef DISABLE_DATA_PROCESSING
            } else if (request->Command == ConfigApi::Modules_StatisticsGet) {
                auto domain = request->getOption(0);
                auto address = request->getOption(1);
                auto propName = request->getOption(2);
                auto resPage = request->getOption(3).toInt();
                auto resPerPage = request->getOption(4).toInt();
                auto module = homeGenie->getModule(&domain, &address);
                if (module != nullptr) {
                    auto parameter = module->getProperty(propName);
                    if (parameter != nullptr) {
                        responseCallback->beginGetLength();
                        homeGenie->writeParameterHistoryJSON(parameter, responseCallback, resPage, resPerPage);
                        responseCallback->endGetLength();
                        homeGenie->writeParameterHistoryJSON(parameter, responseCallback, resPage, resPerPage);
                    }
                }
#endif
            } else if (request->Command == ConfigApi::WebSocket_GetToken) {

                // TODO: implement random token with expiration (like in HG server) for websocket client verification

                responseCallback->writeAll(R"({ "ResponseValue": "e046f885-1d51-4dd2-b952-38e7134a9c0f" })");
                return true;
            } else if (request->Command == ConfigApi::System_Configure) {

                String method = request->getOption(0);
                if (method == ConfigApi::SystemApi::Location_Get) {

                    JsonDocument doc;
                    auto obj = doc.add<JsonObject>();
                    obj["timeZoneId"] = Config::zone.id;
                    obj["timeZone"] = Config::zone.description;
                    obj["name"] = Config::zone.name;
                    obj["utcOffset"] = Config::zone.offset;
                    obj["latitude"] = Config::zone.latitude;
                    obj["longitude"] = Config::zone.longitude;
                    String output;
                    serializeJson(obj, output);

                    responseCallback->writeAll(output.c_str());
                    return true;

                } else if (method == ConfigApi::SystemApi::Location_Set) {
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, request->Data);

                    if (!error.code()) {
                        if (doc.containsKey("name")
                            && doc.containsKey("latitude")
                            && doc.containsKey("longitude")) {

                            Config::zone.id = doc["timeZoneId"].as<String>();
                            Config::zone.description = doc["timeZone"].as<String>();
                            Config::zone.name = doc["name"].as<String>();
                            Config::zone.latitude = doc["latitude"].as<float>();
                            Config::zone.longitude = doc["longitude"].as<float>();
                            if (doc.containsKey("utcOffset")) {
                                Config::zone.offset = doc["utcOffset"].as<int>();
                            }

                            Preferences preferences;
                            preferences.begin(CONFIG_SYSTEM_NAME, false);

                            preferences.putString(CONFIG_KEY_system_zone_id, Config::zone.id);
                            preferences.putString(CONFIG_KEY_system_zone_description, Config::zone.description);
                            preferences.putString(CONFIG_KEY_system_zone_name, Config::zone.name);
                            preferences.putFloat(CONFIG_KEY_system_zone_lat, Config::zone.latitude);
                            preferences.putFloat(CONFIG_KEY_system_zone_lng, Config::zone.longitude);
                            preferences.putInt(CONFIG_KEY_system_zone_offset, Config::zone.offset);

                            preferences.end();

                            // set new timezone
                            Config::updateTimezone();

                            responseCallback->writeAll(ApiHandlerResponseText::OK);
                            return true;
                        }
                    }
                } else if (method == ConfigApi::SystemApi::System_DataSet) {
                    JsonDocument doc;
                    DeserializationError error = deserializeJson(doc, request->Data);

                    if (!error.code()) {
                        int operationsCount = 0;
                        if (doc.containsKey("name")) {
                            String deviceName = doc["name"].as<String>();
                            if (deviceName.isEmpty()) {
                                responseCallback->writeAll(HomeGenieHandlerResponseStatus::ERROR_INVALID_NAME);
                                return true;
                            } else {
                                Config::system.friendlyName = deviceName;
                                miniModule->name = deviceName;
                                // persist setting
                                Preferences preferences;
                                preferences.begin(CONFIG_SYSTEM_NAME, false);
                                preferences.putString(CONFIG_KEY_device_name, deviceName);
                                preferences.end();
                            }
                        }
                        if (doc.containsKey("group")) {
                            String deviceGroup = doc["group"].as<String>();
                            if (deviceGroup.isEmpty()) {
                                responseCallback->writeAll(HomeGenieHandlerResponseStatus::ERROR_INVALID_NAME);
                                return true;
                            } else {
                                Config::saveSetting(CONFIG_KEY_device_group, deviceGroup);
                                operationsCount++;
                            }
                        }
                        if (operationsCount > 0) {
                            responseCallback->writeAll(ApiHandlerResponseText::OK);
                        } else {
                            responseCallback->writeAll(ApiHandlerResponseStatus::ERROR);
                        }
                        return true;
                    }
#ifndef ESP8266
                } else if (method == ConfigApi::SystemApi::System_TimeSet) {

                    String time = request->getOption(1);
                    long seconds = time.substring(0, time.length() - 3).toInt();
                    long ms = time.substring(time.length() - 3).toInt();
                    homeGenie->getNetManager()
                        .getTimeClient()->setTime(seconds, ms);

                    responseCallback->writeAll(ApiHandlerResponseText::OK);
                    return true;
#endif
                } else if (method == ConfigApi::SystemApi::System_Info) {

                    auto info = updateSystemModule();
                    responseCallback->writeAll(info);
                    return true;

                }

                return false;
            }
        } else {
            // Control API implementation for integrated GPIO modules
            uint8_t pinNumber = request->Address.toInt();
            Module* module = getModule(request->Domain.c_str(), request->Address.c_str());
            if (module != nullptr && module != miniModule) {

                auto levelProperty = module->getProperty(IOEventPaths::Status_Level);
                if (levelProperty->value.toFloat() > 0) {
                    GPIOPort::saveLastOnLevel(pinNumber, levelProperty->value.toFloat());
                }

                float level = 0;
                if (request->Command == ControlApi::Control_On) {
                    level = gpioPort->on(pinNumber);
                } else if (request->Command == ControlApi::Control_Off) {
                    level = gpioPort->off(pinNumber);
                } else if (request->Command == ControlApi::Control_Level) {
                    level = gpioPort->level(pinNumber, (uint8_t)request->getOption(0).toFloat());
                } else if (request->Command == ControlApi::Control_Toggle) {
                    if (levelProperty->value.toFloat() == 0) {
                        level = gpioPort->on(pinNumber);
                    } else {
                        level = gpioPort->off(pinNumber);
                    }
                }

                levelProperty->setValue(String(level).c_str());
                GPIOPort::saveLevel(pinNumber, levelProperty->value.toFloat());

                responseCallback->writeAll(ApiHandlerResponseText::OK);
                return  true;
            }
        }
        return false;
    }

    bool HomeGenieHandler::handleEvent(IO::IIOEventSender *sender,
                                       const char* domain, const char* address,
                                       const char *eventPath, void *eventData,
                                       IO::IOEventDataType dataType) {
        auto module = getModule(domain, address);
        if (module) {
            String event = eventPath;
            // Event Stream Message Enqueue (for MQTT/SSE/WebSocket propagation)
            auto m = QueuedMessage(domain, address, eventPath, "", eventData, dataType);
            // Data type handling
            switch (dataType) {
                case Text: {
                    m.value = String((const char *) eventData);
                } break;
                case SensorColorHsv: {
                    auto color = (ColorHSV *) eventData;
                    m.value = String(color->h, 4)
                              + String(",") + String(color->s, 4)
                              + String(",") + String(color->v, 4);
                } break;
                case SensorLight: {
                    m.value = String(*(uint16_t *) eventData);
                } break;
                case SensorTemperature:
                case SensorHumidity:
                case Float: {
                    m.value = String(*(float_t *) eventData);
                } break;
                case UnsignedNumber: {
                    m.value = String(*(uint32_t *) eventData);
                } break;
                case Number:
                default: {
                    m.value = String(*(int32_t *) eventData);
                }
            }
            module->setProperty(event, m.value, eventData, dataType);
            HomeGenie::getInstance()->getEventRouter().signalEvent(m);
        }
        return false;
    }

    Module* HomeGenieHandler::getModule(const char* domain, const char* address) {
        for (int i = 0; i < moduleList.size(); i++) {
            Module* module = moduleList.get(i);
            if (module->domain.equals(domain) && module->address.equals(address))
                return module;
        }
        return nullptr;
    }
    LinkedList<Module*>* HomeGenieHandler::getModuleList() {
        return &moduleList;
    }

    bool HomeGenieHandler::addModule(Module* module) {
        auto m = getModule(module->name.c_str(), module->address.c_str());
        if (m != nullptr) {
            return false;
        }
        moduleList.add(module);
        return true;
    }

    void HomeGenieHandler::getSystemInfoJson(String& output) {
        auto homeGenie = HomeGenie::getInstance();
        JsonDocument doc;
        auto obj = doc.add<JsonObject>();
        obj["Release"] = doc.add<JsonObject>();
        obj["Release"]["Name"] = CONFIG_DEVICE_MODEL_NAME;
        obj["Release"]["Version"] = CONFIG_DEVICE_MODEL_NUMBER;
        obj["Release"]["ReleaseDate"] = ReleaseBuildDate;
        obj["Release"]["Runtime"] = BUILD_ENV_NAME;
        obj["Id"] = Config::system.id;
        obj["Name"] = Config::system.friendlyName;
        obj["Runtime"] = BUILD_ENV_NAME;
        obj["Platform"] = BUILD_PLATFORM;
        obj["TimeZoneId"] = Config::zone.id;
        obj["TimeZone"] = Config::zone.description;
        obj["UtcOffset"] = Config::zone.offset;
        obj["LocalTime"] = homeGenie->getNetManager().getTimeClient()->getNTPClient().getFormattedDate();
        obj["Configuration"] = doc.add<JsonObject>();
        // Default group name
        String defaultGroupName = Config::getSetting(CONFIG_KEY_device_group, "Dashboard");
        obj["Configuration"]["Group"] = defaultGroupName;
        // Location info
        obj["Configuration"]["Location"] = doc.add<JsonObject>();
        obj["Configuration"]["Location"]["name"] = Config::zone.name;
        obj["Configuration"]["Location"]["latitude"] = Config::zone.latitude;
        obj["Configuration"]["Location"]["longitude"] = Config::zone.longitude;
        /*
        // Location sun data
        obj["Configuration"]["Location"]["sunData"] = doc.add<JsonObject>();
        time_t now = time(0);
        auto sd = gmtime(&now);
        // Calculate the times of sunrise, transit, and sunset, in hours (UTC) (relative from midnight of the selected day)
        double relTransit, relSunrise, relSunset;
        calcSunriseSunset(sd->tm_year + 1900, sd->tm_mon + 1, sd->tm_mday, Config::zone.latitude, Config::zone.longitude, relTransit, relSunrise, relSunset);
        // Translate to epoch time
        time_t sunrise = Utility::relativeUtcHoursToLocalTime(relSunrise, now);
        time_t sunset = Utility::relativeUtcHoursToLocalTime(relSunset, now);
        time_t solarNoon = Utility::relativeUtcHoursToLocalTime(relTransit, now);
        obj["Configuration"]["Location"]["sunData"]["Sunrise"] = sunrise;
        obj["Configuration"]["Location"]["sunData"]["Sunset"] = sunset;
        obj["Configuration"]["Location"]["sunData"]["SolarNoon"] = solarNoon;
        */
        serializeJson(obj, output);
    }

    const char* HomeGenieHandler::updateSystemModule() {
        String info;
        getSystemInfoJson(info);
        miniModule->setProperty(ModuleApi::Property::SystemInfo, info);
        return miniModule->getProperty(ModuleApi::Property::SystemInfo)->value.c_str();
    }

}}
