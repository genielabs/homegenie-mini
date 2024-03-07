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

#ifndef HOMEGENIE_MINI_IRTRANSMITTER_H
#define HOMEGENIE_MINI_IRTRANSMITTER_H

#include <HomeGenie.h>

#include <IRutils.h>
#include <IRsend.h>

#include "../configuration.h"
#include "IRTransmitterConfig.h"

namespace IO { namespace IR {

    class IRTransmitterListener {
    public:
        virtual void onSendStart() = 0;
        virtual void onSendComplete() = 0;
    };

    class IRTransmitter: Task {
    public:
        IRTransmitter();
        IRTransmitter(IRTransmitterConfig *);

        void begin();
        void loop() override;
        void sendCommand(String& command, uint8_t repeat, uint16_t delay);
        void setListener(IRTransmitterListener* l);

    private:
        IRTransmitterConfig* configuration;
        IRTransmitterListener* listener = nullptr;
        IRsend* irSend = nullptr;
        void* rawData = nullptr;
        size_t dataSize;
        decode_type_t sendProtocol;
        uint8_t sendCount = 0;
        uint16_t sendDelay = 1000;
        unsigned long lastSentTs = 0;
        void _free();
    };

}}

#endif //HOMEGENIE_MINI_IRTRANSMITTER_H
