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
#include "io/IOEvent.h"
#include "net/NetManager.h"

#define EVENTROUTER_NS_PREFIX            "Service::EventRouter"

namespace Service {

    class QueuedMessage {
    public:
        QueuedMessage() = default;
        explicit QueuedMessage(const String& domain, const String& sender, const String& event, const String& value, void* data = nullptr, IO::IOEventDataType type = IO::Undefined) {
            this->domain = domain;
            this->sender = sender;
            this->event = event;
            this->value = value;
            this->data = data;
            this->type = type;
        }
        explicit QueuedMessage(Data::Module* sender, const String& event, const String& value, void* data = nullptr, IO::IOEventDataType type = IO::Undefined) {
            this->domain = sender->domain;
            this->sender = sender->address;
            this->event = event;
            this->value = value;
            this->data = data;
            this->type = type;
        }
        ~QueuedMessage() = default;
        String domain;
        String sender;
        String event;
        String value;
        void* data{};
        IO::IOEventDataType type = IO::Undefined;
    };

    using namespace Net;

    class EventRouter : Task {
    public:
        void loop() override;
        void signalEvent(const std::shared_ptr<QueuedMessage>& m);

        void withNetManager(NetManager &manager);

    private:
        NetManager* netManager{};
        LinkedList<std::shared_ptr<QueuedMessage>> eventsQueue;
        template <class T>
        bool valueMatchesCondition(String& condition, T leftValue, T rightValue);
    };

}

#endif //HOMEGENIE_MINI_EVENTROUTER_H