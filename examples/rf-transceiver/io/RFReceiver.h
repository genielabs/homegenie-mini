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

#ifndef HOMEGENIE_MINI_RFRECEIVER_H_
#define HOMEGENIE_MINI_RFRECEIVER_H_

#include <RCSwitch.h>

#include <HomeGenie.h>
#include <Utility.h>

#include "RFReceiverConfig.h"

#define RF_RFRECEIVER_NS_PREFIX                  "IO::RF::RfReceiver"

namespace IO { namespace RCS {

    class RFReceiver : Task, public IIOEventSender {
    public:
        RFReceiver();
        RFReceiver(RFReceiverConfig *);

        void begin() override;
        void loop() override;

    private:
        RFReceiverConfig* configuration;
        RCSwitch RF = RCSwitch();
    };

}} // ns

#endif // HOMEGENIE_MINI_RFRECEIVER_H_
