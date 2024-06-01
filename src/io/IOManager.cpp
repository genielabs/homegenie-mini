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

#include "IOManager.h"

namespace IO {

    IOManager::IOManager() {
        systemDiagnostics = new System::Diagnostics();
        systemDiagnostics->setEventReceiver(this);
    }

    void IOManager::begin() {
        for(int i = 0; i < eventSenders.size(); i++) {
            eventSenders[i]->begin();
        }
    }

    bool IOManager::addEventSender(IIOEventSender* sender) {
        eventSenders.add(sender);
        sender->setEventReceiver(this);
        return true;
    }

    void IOManager::setEventReceiver(IIOEventReceiver *receiver) {
        ioEventReceiver = receiver;
    }

    void IOManager::onIOEvent(IIOEventSender *sender, const char* domain, const char* address, const char *eventPath, void *eventData, IOEventDataType dataType) {
        // route event to HomeGenie
        ioEventReceiver->onIOEvent(sender, domain, address, eventPath, eventData, dataType);
    }

}