//
// Created by gene on 11/06/25.
//

#ifndef HOMEGENIE_MINI_AUTODETECTDISPLAY_H
#define HOMEGENIE_MINI_AUTODETECTDISPLAY_H

#include <LGFX_AUTODETECT.hpp>

#ifdef ENABLE_UI

#include "Config.h"
#include "ui/DisplayDriver.h"


namespace UI { namespace Drivers {


    class AutodetectDisplay : public DisplayDriver {
    public:
        AutodetectDisplay();

        bool isRoundDisplay() override {
            return true;
        }

        LGFX_Device* getDisplay() override;

    private:
        LGFX display;

    };


}} // UI::Drivers

#endif // ENABLE_UI

#endif //HOMEGENIE_MINI_AUTODETECTDISPLAY_H
