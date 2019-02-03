//
// Created by gene on 03/02/19.
//

#ifndef HOMEGENIE_MINI_IOMODULE_H
#define HOMEGENIE_MINI_IOMODULE_H

#include <Arduino.h>
#include <net/NetManager.h>

namespace IO {

    using namespace Net;

    class IOModule {
    public:
        uint8_t Type = 0;
        float Level = 0;
        String UpdateTime;
        IOModule() {
            UpdateTime = NetManager::getTimeClient().getFormattedDate();
        }
    };

}


#endif //HOMEGENIE_MINI_IOMODULE_H
