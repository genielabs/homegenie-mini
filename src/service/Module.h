//
// Created by gene on 12/12/23.
//

#ifndef HOMEGENIE_MINI_MODULE_H
#define HOMEGENIE_MINI_MODULE_H

#include "net/NetManager.h"

namespace Service {

    using namespace Net;

    enum ModuleTypes {
        Generic = 0,
        Switch,
        Dimmer,
        Color,
        Sensor
    };

    class ModuleParameter {
    public:
        String name;
        String value;
        String updateTime;

        ModuleParameter() {
            updateTime = NetManager::getTimeClient().getFormattedDate();
        }
        ModuleParameter(String name): ModuleParameter() {
            this->name = name;
        };
        ModuleParameter(String name, String value): ModuleParameter(name) {
            this->value = value;
        };

        bool is(const char* n) const {
            return name.equals(n);
        }
        void setValue(const char* v) {
            value = v;
            updateTime = NetManager::getTimeClient().getFormattedDate();
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
        bool setProperty(String pn, String pv) {
            for(int p = 0; p < properties.size(); p++) {
                auto param = properties.get(p);
                if (param->is(pn.c_str())) {
                    param->setValue(pv.c_str());
                    return true;
                }
            }
            // add new parameter
            properties.add(new ModuleParameter(pn, pv));
            return false;
        }
        ModuleParameter* getProperty(String pn) {
            for(int p = 0; p < properties.size(); p++) {
                auto param = properties.get(p);
                if (param->is(pn.c_str())) {
                    return param;
                }
            }
            return nullptr;
        }
    };

}



#endif //HOMEGENIE_MINI_MODULE_H
