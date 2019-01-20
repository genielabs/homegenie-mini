//
// Created by gene on 19/01/19.
//

#include "Diagnostics.h"

namespace IO { namespace System {

    Diagnostics::Diagnostics() {
        // IEventSender members
        domain = (uint8_t *)IOEventDomains::HomeAutomation_HomeGenie;
        address = (uint8_t *)"mini";
        // update interval
        setLoopInterval(DIAGNOSTICS_SAMPLING_RATE);
    }

    void Diagnostics::loop() {
        uint32_t freeMem = system_get_free_heap_size();
        if (currentFreeMemory != freeMem) {
            Logger::trace("@%s [%s %lu]", DIAGNOSTICS_NS_PREFIX, IOEventPaths::System_BytesFree, freeMem, UnsignedNumber);
            sendEvent((uint8_t*)IOEventPaths::System_BytesFree, &freeMem, UnsignedNumber);
            currentFreeMemory = freeMem;
        }
    }

}}
