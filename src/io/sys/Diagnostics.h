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

namespace IO { namespace System {

    class Diagnostics : Task, public IIOEventSender {
    public:
        Diagnostics();
        void loop();
    };

}}


#endif //HOMEGENIE_MINI_DIAGNOSTICS_H
