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

#ifndef HOMEGENIE_MINI_IOMANAGER_H
#define HOMEGENIE_MINI_IOMANAGER_H

#include <LinkedList.h>

#include "io/IOEvent.h"
#include "io/IOEventDomains.h"
#include "io/IOEventPaths.h"

#include "io/sys/Diagnostics.h"
#include "service/api/APIRequest.h"

namespace IO {

    class IOManager : IIOEventReceiver {
    public:

        IOManager();

        void begin();

        bool addEventSender(IIOEventSender *);

        // IIOEventReceiver interface
        void onIOEvent(IIOEventSender *, const char*, const char*, const char *, void *, IOEventDataType) override;

        void setEventReceiver(IIOEventReceiver *);

    private:
        // Diagnostics
        System::Diagnostics *systemDiagnostics;
        IIOEventReceiver *ioEventReceiver;
        LinkedList<IIOEventSender*> eventSenders;
    };

}

#endif //HOMEGENIE_MINI_IOMANAGER_H
