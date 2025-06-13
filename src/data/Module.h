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
 * - 2023-12-12 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_MODULE_H
#define HOMEGENIE_MINI_MODULE_H

#include <LinkedList.h>

#include "Config.h"
#include "net/TimeClient.h"
#include "io/IOEventData.h"

namespace Data {

    using namespace IO;

    class ModuleParameter {
    public:
        class UpdateListener {
        public:
            virtual void onUpdate(ModuleParameter* option) = 0;
        };
        String name;
        String value;
        String updateTime;
        void* data{};
        IOEventDataType dataType = IOEventDataType::Undefined;

        ModuleParameter() {
            updateTime = getFormattedDate();
        }
        explicit ModuleParameter(const String& name): ModuleParameter() {
            this->name = name;
        };
        ModuleParameter(const String& name, const String& value): ModuleParameter(name) {
            setValue(value.c_str());
        };

        bool is(const char* n) const {
            return name.equals(n);
        }
        void setValue(const char* v) {
            value = v;
            updateTime = getFormattedDate();
            // save value if config key is set
            if (!configKey.isEmpty()) {
                Config::saveSetting(configKey.c_str(), value);
            }
            if (updateListeners.size() > 0) {
                for (auto l : updateListeners) {
                    l->onUpdate(this);
                }
            }
        }
        void setData(void* d, IOEventDataType t) {
            data = d;
            dataType = t;
        }

        ModuleParameter* addUpdateListener(UpdateListener* listener) {
            updateListeners.add(listener);
            return this;
        }
        UpdateListener* removeUpdateListener(UpdateListener* listener) {
            unsigned int i = 0;
            for (auto l : updateListeners) {
                if (l == listener) {
                    return updateListeners.remove(i);
                }
                i++;
            }
            return nullptr;
        }
        ModuleParameter* withConfigKey(const char* key) {
            configKey = key;
            if (!configKey.isEmpty()) {
                // load stored value
                setValue(Config::getSetting(key, value.c_str()).c_str());
            }
            return this;
        }
        String& getConfigKey() {
            return configKey;
        }
    private:
        LinkedList<UpdateListener*> updateListeners;
        static String getFormattedDate() {
            return Net::TimeClient::getNTPClient().getFormattedDate();
        }
        String configKey = "";
    };

    class ModuleReference {
    public:
        String serviceId;
        String domain;
        String address;
        ModuleReference(const String& d, const String& a) {
            serviceId = "";
            domain = d;
            address = a;
        }
        ModuleReference(const char* d, const char* a) {
            serviceId = "";
            domain = d;
            address = a;
        }
        ModuleReference(const String& s, const String& d, const String& a) {
            serviceId = s;
            domain = d;
            address = a;
        }
        ModuleReference(const char* s, const char* d, const char* a) {
            serviceId = s;
            domain = d;
            address = a;
        }
    };

    class Module {
    public:
        String domain;
        String address;
        String type;
        String name;
        String description;
        LinkedList<ModuleParameter*> properties;

        std::function<void(const char* oldValue, const char* newValue)> onNameUpdated = nullptr;
        std::function<void(const char* oldValue, const char* newValue)> onDescriptionUpdated = nullptr;
        std::function<void(const char* oldValue, const char* newValue)> onTypeUpdated = nullptr;

        bool setProperty(const String& pn, const String& pv, void* data = nullptr, IOEventDataType dataType = Undefined);
        ModuleParameter* getProperty(const String& pn) const {
            for(int p = 0; p < properties.size(); p++) {
                auto param = properties.get(p);
                if (param != nullptr && param->is(pn.c_str())) {
                    return param;
                }
            }
            return nullptr;
        }
        ModuleReference* getReference() const {
            return new ModuleReference(Config::system.id, domain, address);
        }

        ModuleParameter* addWidgetOption(const char* optionName, const char* value, const char* definition) {
            // Add the actual property where the UI control read/write values from/to.
            auto optionParameter = new ModuleParameter(optionName, value);
            properties.add(optionParameter);
            // Add the property where the definition of this option UI control is stored
            auto widgetOption = String("Widget.OptionField.") + String(optionName);
            properties.add(new ModuleParameter(widgetOption, definition));
            return optionParameter;
        }
    };

}

#endif //HOMEGENIE_MINI_MODULE_H
