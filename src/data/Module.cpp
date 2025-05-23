//
// Created by gene on 12/12/23.
//

#include "Module.h"

namespace Data {

    bool Module::setProperty(const String &pn, const String& pv, void *data, IOEventDataType dataType) {
        for(int p = 0; p < properties.size(); p++) {
            auto param = properties.get(p);
            if (param->is(pn.c_str())) {
                param->setValue(pv.c_str());
                param->setData(data, dataType);
                return true;
            }
        }
        // add new parameter
        properties.add(new ModuleParameter(pn, pv));
        return false;
    }

}