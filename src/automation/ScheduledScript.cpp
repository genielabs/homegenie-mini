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
 * - 2019-01-13 Initial release
 *
 */

#include "ScheduledScript.h"

#ifndef DISABLE_AUTOMATION

namespace Automation {

    static const char baseCode[] PROGMEM = "const $$ = {\n"
            "  get net() {\n"
            "    _url = '';\n"
            "    _username = '';\n"
            "    _password = '';\n"
            "    _command = '';\n"
            "    _getUrl = function() {\n"
            "      if (!_username || _username.trim() === '') {\n"
            "        return _url;\n"
            "      }\n"
            "      const encUser = encodeURIComponent(_username);\n"
            "      const encPass = _password ? encodeURIComponent(_password) : '';\n"
            "      credentials = encUser;\n"
            "      if (encPass) {\n"
            "        credentials += ':' + encPass;\n"
            "      }\n"
            "      const separator = '://';\n"
            "      const separatorIndex = _url.indexOf(separator);\n"
            "      if (separatorIndex === -1) {\n"
            "        return _url;\n"
            "      }\n"
            "      const protocol = _url.substring(0, separatorIndex + separator.length);\n"
            "      const restOfUrl = _url.substring(separatorIndex + separator.length);\n"
            "      return protocol + credentials + '@' + restOfUrl;\n"
            "    };\n"
            "    return {\n"
            "      webService: function(url) {\n"
            "        _url = url;\n"
            "        return this;\n"
            "      },\n"
            "      withCredentials: function(user, pass) {\n"
            "        _username = user;\n"
            "        _password = pass;"
            "        return this;\n"
            "      },\n"
            "      call: function() {\n"
            "        return __netHelper_call(_getUrl());\n"
            "      },\n"
            "      post: function(data) {\n"
            "        return __netHelper_post(_getUrl(), data);\n"
            "      },\n"
            "      ping: function(host) {\n"
            "        return __netHelper_ping(host);\n"
            "      }\n"
            "    }\n"
            "  },\n"
            "  get boundModules() {\n"
            "    return {\n"
            "      command: function(cmd) {\n"
            "        _command = cmd;"
            "        return this;\n"
            "      },\n"
            "      submit: function(opts) {\n"
            "        __boundModules_command(_command, opts);\n"
            "        return this;\n"
            "      },"
            "      on: function() {\n"
            "        __boundModules_command('Control.On', '');\n"
            "        return this;\n"
            "      },\n"
            "      off: function() {\n"
            "        __boundModules_command('Control.Off', '');\n"
            "        return this;\n"
            "      },\n"
            "      toggle: function() {\n"
            "        __boundModules_command('Control.Toggle', '');\n"
            "        return this;\n"
            "      },\n"
            "      set level(level) {\n"
            "        __boundModules_command('Control.Level', +level * 100);\n"
            "        return this;\n"
            "      },\n"
            "      get level() {\n"
            "        return __boundModules_property_avg('Status.Level');\n"
            "      },\n"
            "      set colorHsb(hsb) {\n"
            "        __boundModules_command('Control.ColorHsb', hsb);\n"
            "        return this;\n"
            "      },\n"
            "      get colorHsb() {\n"
            "        return __boundModules_property_get('Status.ColorHsb');\n"
            "      },\n"
            "      get isOn() {\n"
            "        return parseFloat(__boundModules_property_avg('Status.Level')) > 0;\n"
            "      },\n"
            "      get isOff() {\n"
            "        return parseFloat(__boundModules_property_avg('Status.Level')) === 0;\n"
            "      },\n"
            "      get alarmed() {"
            "        return parseInt(__boundModules_property_avg('Sensor.Alarm')) > 0;"
            "      },"
            "      get motionDetected() {"
            "        return parseInt(__boundModules_property_avg('Sensor.MotionDetect')) > 0;"
            "      },"
            "      get temperature() {\n"
            "        return parseFloat(__boundModules_property_avg('Sensor.Temperature'));\n"
            "      },\n"
            "      get luminance() {\n"
            "        return parseFloat(__boundModules_property_avg('Sensor.Luminance'));\n"
            "      },\n"
            "      get humidity() {\n"
            "        return parseFloat(__boundModules_property_avg('Sensor.Humidity'));\n"
            "      }\n"
            "    }\n"
            "  },\n"
            "  onPrevious: function() {\n"
            "    return __onPrevious();\n"
            "  },\n"
            "  onNext: function() {\n"
            "    return __onNext();\n"
            "  },\n"
            "  data: function(k, v) {\n"
            "    __log(k, v);\n" // TODO: cross-instance persistent data(<key>, <value>) 2B implemented
            "  },\n"
            "  log: function(k, v) {\n"
            "    __log(k, v);\n"
            "  },\n"
            "  pause: function(seconds) {\n"
            "    __pause(seconds);\n"
            "    return this;\n"
            "  }\n"
            "};\n";

    ScheduledScript::ScheduledScript(Schedule *s) {
        schedule = s;
    }

    void ScheduledScript::run() {

        if (schedule->script.isEmpty()) {
            return;
        }

        Logger::info(":%s [Scheduler::Event] >> ['%s' running]", PROGRAMENGINE_NS_PREFIX,
                     schedule->name.c_str());

        duk_context *ctx = duk_create_heap_default();

        duk_push_c_lightfunc(ctx, helper_log, DUK_VARARGS, 0, 0);
        duk_put_global_string(ctx, "__log");

        duk_push_c_lightfunc(ctx, pause, 1, 1, 0);
        duk_put_global_string(ctx, "__pause");

        duk_push_c_lightfunc(ctx, schedule_on_previous, 1, 1, 0);
        duk_put_global_string(ctx, "__onPrevious");

        duk_push_c_lightfunc(ctx, schedule_on_next, 1, 1, 0);
        duk_put_global_string(ctx, "__onNext");

        duk_push_c_lightfunc(ctx, netHelper_call, 1, 1, 0);
        duk_put_global_string(ctx, "__netHelper_call");

        duk_push_c_lightfunc(ctx, netHelper_post, 2, 2, 0);
        duk_put_global_string(ctx, "__netHelper_post");

        duk_push_c_lightfunc(ctx, netHelper_ping, 1, 1, 0);
        duk_put_global_string(ctx, "__netHelper_ping");

        duk_push_c_lightfunc(ctx, boundModules_command, 2, 2, 0);
        duk_put_global_string(ctx, "__boundModules_command");

        duk_push_c_lightfunc(ctx, boundModules_property_avg, 1, 1, 0);
        duk_put_global_string(ctx, "__boundModules_property_avg");

        duk_push_c_lightfunc(ctx, boundModules_property_get, 1, 1, 0);
        duk_put_global_string(ctx, "__boundModules_property_get");

        duk_push_pointer(ctx, schedule);
        duk_put_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));

        auto event = schedule->getLastEvent();
        if (event != nullptr) {
            auto obj_idx = duk_push_object(ctx);
            duk_push_string(ctx, event->domain.c_str());
            duk_put_prop_string(ctx, obj_idx, "domain");
            duk_push_string(ctx, event->sender.c_str());
            duk_put_prop_string(ctx, obj_idx, "address");
            duk_push_string(ctx, event->event.c_str());
            duk_put_prop_string(ctx, obj_idx, "property");
            duk_push_string(ctx, event->value.c_str());
            duk_put_prop_string(ctx, obj_idx, "value");
            duk_put_global_string(ctx, "event");
            schedule->setLastEvent(nullptr);
        }

        const String scriptCode = baseCode + schedule->script;

        duk_peval_string(ctx, scriptCode.c_str());
        //duk_pop(ctx);  // pop eval result

        duk_destroy_heap(ctx);
    }

    duk_ret_t ScheduledScript::pause(duk_context *ctx) {
        double pauseMs = (1000.0F * duk_to_number(ctx, 0));
#ifdef CONFIG_AUTOMATION_SPAWN_FREERTOS_TASK
        vTaskDelay(portTICK_PERIOD_MS * pauseMs);
#else
        unsigned long start = millis();
        while (millis() - start < pauseMs) {
            TaskManager::loop();
        }
#endif
        return 0;
    }

    duk_ret_t ScheduledScript::schedule_on_previous(duk_context *ctx) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        bool result = schedule->info.onPreviousMinute > 0;
        duk_push_boolean(ctx, result);
        return 1;
    }

    duk_ret_t ScheduledScript::schedule_on_next(duk_context *ctx) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        bool result = schedule->info.onNextMinute > 0;
        duk_push_boolean(ctx, result);
        return 1;
    }

    duk_ret_t ScheduledScript::boundModules_property_avg(duk_context *ctx) {
        String propertyName = duk_to_string(ctx, 0);
        float res = getAvgPropertyValue(ctx, propertyName.c_str());
        duk_push_number(ctx, res);
        return 1;
    }

    duk_ret_t ScheduledScript::boundModules_property_get(duk_context *ctx) {
        String propertyName = duk_to_string(ctx, 0);
        const char* res = getProperty(ctx, propertyName.c_str());
        duk_push_string(ctx, res);
        return 1;
    }

    duk_ret_t ScheduledScript::boundModules_command(duk_context *ctx) {
        String command = duk_to_string(ctx, 0);
        String options = duk_to_string(ctx, 1);
        if (options == nullptr || options == "undefined") options = "";
        apiCommand(ctx, command.c_str(), options.c_str());
        return 0;
    }

    duk_ret_t ScheduledScript::netHelper_call(duk_context *ctx) {
        String url = duk_to_string(ctx, 0);
        String response = Helpers::NetHelper::httpGet(url);
        duk_push_string(ctx, response.c_str());
        return 1;
    }

    duk_ret_t ScheduledScript::netHelper_post(duk_context *ctx) {
        String url = duk_to_string(ctx, 0);
        String data = duk_to_string(ctx, 1);
        String response = Helpers::NetHelper::httpPost(url, data);
        duk_push_string(ctx, response.c_str());
        return 1;
    }

    duk_ret_t ScheduledScript::netHelper_ping(duk_context *ctx) {
        String host = duk_to_string(ctx, 0);
        bool result = Helpers::NetHelper::ping(host);
        duk_push_boolean(ctx, result);
        return 1;
    }

    float ScheduledScript::getAvgPropertyValue(duk_context *ctx, const char* propertyPath) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        int vc = 0; float avg = 0;
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        for (auto mr: schedule->boundModules) {
            auto module = HomeGenie::getInstance()->getModule(&mr->domain, &mr->address);
            if (module != nullptr) {
                auto property = module->getProperty(propertyPath);
                if (property != nullptr && property->value != nullptr) {
                    avg += property->value.toFloat();
                    vc++;
                }
            }
        }
        return avg / (float)vc;
    }

    const char* ScheduledScript::getProperty(duk_context *ctx, const char* propertyPath) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        for (auto mr: schedule->boundModules) {
            auto module = HomeGenie::getInstance()->getModule(&mr->domain, &mr->address);
            if (module != nullptr) {
                auto property = module->getProperty(propertyPath);
                if (property != nullptr && property->value != nullptr) {
                    return property->value.c_str();
                }
            }
        }
        return "";
    }

    void ScheduledScript::apiCommand(duk_context *ctx, const char* command, const char* options) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        for (auto mr: schedule->boundModules) {
            String apiCommand = String("/api/") + mr->domain + String("/") + mr->address + String("/") + String(command) + String("/") + String(options);

            if (mr->serviceId.indexOf("://") > 0) {
                if (mr->serviceId.endsWith("/")) {
                    apiCommand = mr->serviceId + apiCommand.substring(1);
                } else {
                    apiCommand = mr->serviceId + apiCommand;
                }
                Helpers::NetHelper::httpGet(apiCommand);
            } else {
                auto callback = DummyResponseCallback();
                ProgramEngine::apiRequest(schedule, apiCommand.c_str(), &callback);
            }

        }
    }

    duk_ret_t ScheduledScript::helper_log(duk_context *ctx) {
        String res = "";
        int n = duk_get_top(ctx);  // #args
        for (int i = 0; i < n; i++) {
            res += duk_to_string(ctx, i);
        }
        Serial.println(res);
        return 0;
    }

}

#endif