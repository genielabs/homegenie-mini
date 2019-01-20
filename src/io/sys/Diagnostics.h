//
// Created by gene on 19/01/19.
//

#ifndef HOMEGENIE_MINI_DIAGNOSTICS_H
#define HOMEGENIE_MINI_DIAGNOSTICS_H

extern "C" {
#include "user_interface.h"
}

#include <Task.h>
#include <io/Logger.h>
#include <io/IOEvent.h>
#include <io/IOEventDomains.h>
#include <io/IOEventPaths.h>

#define DIAGNOSTICS_NS_PREFIX          "IO::Sys::Diagnostics"
#define DIAGNOSTICS_SAMPLING_RATE       5000L

namespace IO { namespace System {

    class Diagnostics : Task, public IIOEventSender {
    public:
        Diagnostics();
        void loop();

    private:
        uint32_t currentFreeMemory;
    };

}}


#endif //HOMEGENIE_MINI_DIAGNOSTICS_H
