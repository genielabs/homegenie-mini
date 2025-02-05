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
 * - 2019-01-19 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_IIOEVENT_H
#define HOMEGENIE_MINI_IIOEVENT_H

#include "Config.h"

#include "data/Module.h"
#ifndef DISABLE_DATA_PROCESSING
#include "data/Statistics.h"
#endif
#include "IOEventData.h"
#include "IOEventPaths.h"

namespace IO {

    using namespace Data;
#ifndef DISABLE_DATA_PROCESSING
    using namespace Data::Processing;
#endif

    class IIOEventSender;

    // IIOEventReceiver interface
    class IIOEventReceiver {
    public:
        virtual void onIOEvent(IIOEventSender *, const char *, const char *, const char *, void *, IOEventDataType dataType) = 0; // pure virtual
    };

    // IIOEventSender interface
    class IIOEventSender {
    public:
        virtual void begin() = 0;
        void setEventReceiver(IIOEventReceiver *receiver) {
            eventReceiver = receiver;
        }

        virtual void setModule(Module* m) {
            module = m;
        }
        virtual void sendEvent(const char *domain, const char *address, const char *eventPath, void *eventData, IOEventDataType dataType) {
            if (eventReceiver != nullptr) {
                eventReceiver->onIOEvent(this, domain, address, eventPath, eventData, dataType);
            }
        };
        virtual void sendEvent(const char *eventPath, void *eventData, IOEventDataType dataType) {
            if (eventReceiver != nullptr && module != nullptr) {
                auto eventsDisable = module->getProperty(IOEventPaths::Events_Disabled);
                if (eventsDisable == nullptr || eventsDisable->value == nullptr || eventsDisable->value != "1") {
                    eventReceiver->onIOEvent(this, module->domain.c_str(), module->address.c_str(), eventPath, eventData, dataType);
                }
#ifndef DISABLE_DATA_PROCESSING
                if (isNumericDataType(dataType)) {
                    auto moduleParameter = module->getProperty(eventPath);
                    if (moduleParameter != nullptr && !moduleParameter->name.startsWith("System.")) {
                        Statistics::collect(moduleParameter);
                    }
                }
#endif
            }
        };

    protected:
        IIOEventReceiver *eventReceiver = nullptr;
        Module* module = nullptr;
    };

}

#endif //HOMEGENIE_MINI_IIOEVENT_H
