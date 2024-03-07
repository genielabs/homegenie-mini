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

#ifndef HOMEGENIE_MINI_IRRECEIVER_H_
#define HOMEGENIE_MINI_IRRECEIVER_H_

#include <HomeGenie.h>
#include <Utility.h>

#include <IRrecv.h>
#include <IRutils.h>

#include "IRReceiverConfig.h"

#define IR_IRRECEIVER_NS_PREFIX                  "IO::IR::IrReceiver"

namespace IO { namespace IR {

    class IRReceiver : Task, public IIOEventSender {
    public:
        IRReceiver();
        IRReceiver(IRReceiverConfig *);

        void begin() override;
        void loop() override;

        bool enabled();
        void enabled(bool);

    private:
        IRReceiverConfig *configuration;
        IRrecv* irReceiver = nullptr;
        decode_results results;  // Somewhere to store the results
        bool isEnabled = false;
    };

}} // ns

#endif // HOMEGENIE_MINI_IRRECEIVER_H_
