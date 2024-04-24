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
 * - 2019-01-13 Initial release
 *
 */

#include "ScheduledScript.h"

#ifndef DISABLE_AUTOMATION

namespace Automation {

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

        duk_push_c_lightfunc(ctx, helper_log, 2, 2, 0);
        duk_put_global_string(ctx, "__log");

        duk_push_c_lightfunc(ctx, pause, 1, 1, 0);
        duk_put_global_string(ctx, "__pause");

        duk_push_c_lightfunc(ctx, boundModules_on, 0, 0, 0);
        duk_put_global_string(ctx, "__boundModules_on");

        duk_push_c_lightfunc(ctx, boundModules_off, 0, 0, 0);
        duk_put_global_string(ctx, "__boundModules_off");

        duk_push_c_lightfunc(ctx, boundModules_toggle, 0, 0, 0);
        duk_put_global_string(ctx, "__boundModules_toggle");

        duk_push_c_lightfunc(ctx, boundModules_level_set, 1, 1, 0);
        duk_put_global_string(ctx, "__boundModules_level_set");

        duk_push_c_lightfunc(ctx, boundModules_colorHsb_set, 1, 1, 0);
        duk_put_global_string(ctx, "__boundModules_colorHsb_set");


        duk_push_pointer(ctx, schedule);
        duk_put_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));


        String scriptCode = "const $$ = {"
                            "  boundModules: {"
                            "    on: function() {"
                            "      __boundModules_on();"
                            "    },"
                            "    off: function() {"
                            "      __boundModules_off();"
                            "    },"
                            "    toggle: function() {"
                            "      __boundModules_toggle();"
                            "    },"
                            "    set level(v) {"
                            "      __boundModules_level_set(v);"
                            "    },"
                            "    get level() {"
                            "      return '';" // TODO: implement getter
                            "    },"
                            "    set colorHsb(hsb) {"
                            "      __boundModules_colorHsb_set(hsb);"
                            "    },"
                            "    get colorHsb() {"
                            "      return '';" // TODO: implement getter
                            "    }"
                            "  },"
                            "  onPrevious: function() {"
                            "     return false;" // TODO: 2B implemented
                            "  },"
                            "  onNext: function() {"
                            "     return false;" // TODO: 2B implemented
                            "  },"
                            "  data: function(k, v) {"
                            "    __log(k, v);" // TODO: 2B implemented
                            "  },"
                            "  pause: function(seconds) {"
                            "    __pause(seconds);"
                            "  }"
                            "};" +
                            schedule->script +
                            "\n;";


        duk_peval_string(ctx, scriptCode.c_str());
        duk_pop(ctx);  // pop eval result

        duk_destroy_heap(ctx);
    }

    duk_ret_t ScheduledScript::pause(duk_context *ctx) {
        double pauseMs = (1000.0F * duk_to_number(ctx, 0));
#ifdef CONFIG_CREATE_AUTOMATION_TASK
        vTaskDelay(portTICK_PERIOD_MS * pauseMs);
#else
        unsigned long start = millis();
        while (millis() - start < pauseMs) {
            TaskManager::loop();
        }
#endif
        return 0;
    }

    duk_ret_t ScheduledScript::boundModules_level_get(duk_context *ctx) {
        const char* res = getProperty(ctx, IOEventPaths::Status_Level);
        duk_push_string(ctx, res);
        return 1;
    }

    duk_ret_t ScheduledScript::boundModules_level_set(duk_context *ctx) {
        String level = duk_to_string(ctx, 0);
        auto command = String(ControlApi::Control_Level) + String("/") + String(level);
        apiCommand(ctx, command.c_str());
        return 0;
    }

    duk_ret_t ScheduledScript::boundModules_colorHsb_get(duk_context *ctx) {
        const char* res = getProperty(ctx, IOEventPaths::Status_ColorHsb);
        duk_push_string(ctx, res);
        return 1;
    }

    duk_ret_t ScheduledScript::boundModules_colorHsb_set(duk_context *ctx) {
        String hsb = duk_to_string(ctx, 0);
        auto command = String(ControlApi::Control_ColorHsb) + String("/") + String(hsb);
        apiCommand(ctx, command.c_str());
        return 0;
    }

    duk_ret_t ScheduledScript::boundModules_on(duk_context *ctx) {
        apiCommand(ctx, ControlApi::Control_On);
        return 0;
    }

    duk_ret_t ScheduledScript::boundModules_off(duk_context *ctx) {
        apiCommand(ctx, ControlApi::Control_Off);
        return 0;
    }

    duk_ret_t ScheduledScript::boundModules_toggle(duk_context *ctx) {
        apiCommand(ctx, ControlApi::Control_Toggle);
        return 0;
    }

    const char* ScheduledScript::getProperty(duk_context *ctx, const char* propertyPath) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        for (auto mr: schedule->boundModules) {
            auto module = HomeGenie::getInstance()->getModule(&mr->domain, &mr->address);
            if (module != nullptr) {
                auto property = module->getProperty(IOEventPaths::Status_ColorHsb);
                if (property != nullptr && property->value != nullptr) {
                    return property->value.c_str();
                }
            }
        }
        return "";
    }

    void ScheduledScript::apiCommand(duk_context *ctx, const char* command) {
        duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        auto schedule = (Schedule*)duk_get_pointer(ctx, -1);
        for (auto mr: schedule->boundModules) {
            String apiCommand = String("/api/") + mr->domain + String("/") + mr->address + String("/") + String(command);

            // TODO: add support for generic HTTP commands

            auto callback = DummyResponseCallback();
            ProgramEngine::apiRequest(schedule, apiCommand.c_str(), &callback);
        }
    }

    duk_ret_t ScheduledScript::helper_log(duk_context *ctx) {
        String res = "";
        int n = duk_get_top(ctx);  // #args
        for (int i = 0; i < n; i++) {
            res += String(":") + duk_to_string(ctx, i);
        }

        //duk_get_global_string(ctx, DUK_HIDDEN_SYMBOL("schedule"));
        //auto s = (Schedule*)duk_get_pointer(ctx, -1);

        duk_push_string(ctx, res.c_str());
        return 1;
    }

}

#endif