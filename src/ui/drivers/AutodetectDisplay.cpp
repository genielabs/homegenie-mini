//
// Created by gene on 11/06/25.
//

#ifdef ENABLE_UI

#include "AutodetectDisplay.h"


namespace UI { namespace Drivers {

    AutodetectDisplay::AutodetectDisplay() {
        display.init();
    }

    LGFX_Device *AutodetectDisplay::getDisplay() {
        return &display;
    };

}}


#endif // ENABLE_UI