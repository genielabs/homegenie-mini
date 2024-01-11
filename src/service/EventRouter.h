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
 * - 2019-01-23 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_EVENTROUTER_H
#define HOMEGENIE_MINI_EVENTROUTER_H

#include <LinkedList.h>
#include <MsgPack.h>

#include "Task.h"
#include "net/NetManager.h"

#define EVENTROUTER_NS_PREFIX            "Service::EventRouter"

namespace Service {

    class QueuedMessage {
    public:
        QueuedMessage(){};
        QueuedMessage(String domain, String sender, String event, String value) {
            this->domain = domain;
            this->sender = sender;
            this->event = event;
            this->value = value;
        }
        ~QueuedMessage() {
        }
        String domain;
        String sender;
        String event;
        String value;
    };

    using namespace Net;

    class EventRouter : Task {
    public:
        void loop();
        void signalEvent(QueuedMessage m);

        void withNetManager(NetManager &manager);

    private:
        NetManager* netManager;
        LinkedList<QueuedMessage> eventsQueue;
    };

}

#endif //HOMEGENIE_MINI_EVENTROUTER_H