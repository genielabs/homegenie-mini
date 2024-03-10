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
 * - 2019-01-13 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_APIREQUEST_H
#define HOMEGENIE_MINI_APIREQUEST_H

#include "Config.h"
#include "io/Logger.h"

#define APIREQUEST_LOG_PREFIX           "@Service::ApiRequest"


namespace Service { namespace API {

    namespace ControlApi {
        // generic switch / light
        static const char Control_On[] PROGMEM = {"Control.On"};
        static const char Control_Off[] PROGMEM = {"Control.Off"};
        static const char Control_Toggle[] PROGMEM = {"Control.Toggle"};
        static const char Control_Level[] PROGMEM = {"Control.Level"};
        static const char Control_ColorHsb[] PROGMEM = {"Control.ColorHsb"};
        // shutters / door locks
        static const char Control_Open[] PROGMEM = {"Control.Open"};
        static const char Control_Close[] PROGMEM = {"Control.Close"};
    }

    namespace ConfigApi {
        static const char Modules_List[] PROGMEM = {"Modules.List"};
        static const char Modules_Get[] PROGMEM = {"Modules.Get"};
        static const char Modules_ParameterSet[] PROGMEM = {"Modules.ParameterSet"};
        static const char Groups_List[] PROGMEM = {"Groups.List"};
        static const char WebSocket_GetToken[] PROGMEM = {"WebSocket.GetToken"};
    }

    class APIRequest {
    public:
        String Prefix;
        String Domain;
        String Address;
        String Command;
        String OptionsString;
        String getOption(unsigned int optionIndex);
        static APIRequest parse(String command);
    };

}}

#endif //HOMEGENIE_MINI_APIREQUEST_H
