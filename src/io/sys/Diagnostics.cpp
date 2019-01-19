//
// Created by gene on 19/01/19.
//

#include <io/IOEventDomains.h>
#include "Diagnostics.h"

namespace IO { namespace System {

    Diagnostics::Diagnostics() {
        // IEventSender members
        domain = (uint8_t *)IOEventDomains::HomeAutomation_HomeGenie;
        address = (uint8_t *)"SYS";
        // update interval
        setLoopInterval(5000);
    }

    void Diagnostics::loop() {
        uint32_t freeMem = system_get_free_heap_size();
        Logger::trace("@IO::Diagnostics::Memory.BytesFree %lu", freeMem);
        sendEvent((uint8_t*)"System.BytesFree", (void*)freeMem);
    }

}}
