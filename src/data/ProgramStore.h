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
 */

#ifndef HOMEGENIE_MINI_PROGRAMSTORE_H
#define HOMEGENIE_MINI_PROGRAMSTORE_H


#include "JsonStore.h"
#include "Module.h"
#include "io/IOEventDomains.h"

namespace Data {

    using namespace Data;

    namespace ProgramField {
        static const char address[] = "Address";
        static const char name[] = "Name";
        static const char description[] = "Description";
        static const char isEnabled[] = "IsEnabled";
        static const char properties[] = "Properties";
    }
    namespace ProgramData {
        static const char fileName[] = "/programs.json";
    }

    class Program: public Module {
    public:
        Program(const char* address, const char* name, const char* description) {
            this->domain = IOEventDomains::HomeAutomation_HomeGenie_Automation;
            this->address = address;
            this->name = name;
            this->description = description;
        }
        bool isEnabled = false;
    };

    class ProgramStatusListener {
    public:
        // TODO: virtual void onProgramStopped() = 0;
        // TODO: virtual void onProgramStarted() = 0;
        virtual void onProgramEnabled(Program* p) = 0;
        virtual void onProgramDisabled(Program* p) = 0;
    };

    class ProgramStore: public JsonStore<Program> {
    public:
        ProgramStore() : JsonStore<Program>() {
            fileName = ProgramData::fileName;
        }

        void enable(Program* program) {
            if (program!= nullptr && statusCallback != nullptr) {
                program->isEnabled = true;
                statusCallback->onProgramEnabled(program);
            }
        }
        void disable(Program* program) {
            if (program!= nullptr && statusCallback != nullptr) {
                program->isEnabled = false;
                statusCallback->onProgramDisabled(program);
            }
        }

        void setStatusCallback(ProgramStatusListener* callback) {
            statusCallback = callback;
        }


        // Adds or updates
        void addItem(Program* program) override {
            int existingIndex = getItemIndex(program->address.c_str());
            if (existingIndex != -1) {
                existingIndex = existingIndex;
                itemList.remove(existingIndex);
            }
            itemList.add(existingIndex, program);
            save();
        }
        Program* getItem(const char* address) {
            int idx = getItemIndex(address);
            return idx != -1 ? itemList.get(idx) : nullptr;
        }
        int getItemIndex(const char* address) {
            for (int i = 0; i < itemList.size(); i++) {
                auto s = itemList.get(i);
                if (s->address == address) {
                    return i;
                }
            }
            return -1;
        }

        Program* getItemFromJson(JsonVariant& json) override {

            auto program = new Program(
                    json[ProgramField::address].as<const char*>(),
                    json[ProgramField::name].as<const char*>(),
                    json[ProgramField::description].as<const char*>()
            );

            if (json.containsKey(ProgramField::isEnabled)) {
                program->isEnabled = json[ProgramField::isEnabled].as<bool>();
            }

            if (json.containsKey(ProgramField::properties)) {
                auto properties = json[ProgramField::properties].as<JsonArray>();
                for (auto jsonProperty: properties) {
                    auto p = jsonProperty.as<JsonObject>();
                    auto moduleProperty = parseModuleParameter(p);
                    program->properties.add(moduleProperty);
                }
            }

            return program;
        }
        void getJson(JsonObject* jsonItem, Program* item) override {
            auto s = (*jsonItem);
            s[ProgramField::isEnabled] = item->isEnabled;
            s[ProgramField::address] = item->address;
            s[ProgramField::name] = item->name;
            s[ProgramField::description] = item->description;
            JsonArray properties = s[ProgramField::properties].to<JsonArray>();
            for (int b = 0; b < item->properties.size(); b++) {
                auto p = item->properties.get(b);
                JsonObject mr = properties.add<JsonObject>();
                mr["Name"] = p->name;
                mr["Value"] = p->value;
            }
        }

        static ModuleParameter* parseModuleParameter(JsonObject& json) {
            String name = json["Name"].as<const char *>();
            String value = json["Value"].as<const char *>();
            return new ModuleParameter(name, value);
        }

    private:
        ProgramStatusListener* statusCallback = nullptr;
    };

}

#endif //HOMEGENIE_MINI_PROGRAMSTORE_H
