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
        static const char Control_On[] = {"Control.On"};
        static const char Control_Off[] = {"Control.Off"};
        static const char Control_Toggle[] = {"Control.Toggle"};
        static const char Control_Level[] = {"Control.Level"};
        static const char Control_ColorHsb[] = {"Control.ColorHsb"};
        // shutters / door locks
        static const char Control_Open[] = {"Control.Open"};
        static const char Control_Close[] = {"Control.Close"};
    }

    namespace AutomationApi {
        static const char Scheduling_Add[] = {"Scheduling.Add"};
        static const char Scheduling_Update[] = {"Scheduling.Update"};
        static const char Scheduling_Get[] = {"Scheduling.Get"};
        static const char Scheduling_ModuleUpdate[] = {"Scheduling.ModuleUpdate"};
        static const char Scheduling_ListOccurrences[] = {"Scheduling.ListOccurrences"};
        static const char Scheduling_Enable[] = {"Scheduling.Enable"};
        static const char Scheduling_Disable[] = {"Scheduling.Disable"};
        static const char Scheduling_Delete[] = {"Scheduling.Delete"};
        static const char Scheduling_List[] = {"Scheduling.List"};
        static const char Scheduling_Templates[] = {"Scheduling.Templates"};
    }

    namespace ConfigApi {
        static const char Modules_List[] = {"Modules.List"};
        static const char Modules_Get[] = {"Modules.Get"};
        static const char Modules_ParameterSet[] = {"Modules.ParameterSet"};
        static const char Groups_List[] = {"Groups.List"};
        static const char WebSocket_GetToken[] = {"WebSocket.GetToken"};
        static const char System_Configure[] = {"System.Configure"};
        namespace SystemApi {
            static const char Location_Get[] = {"Location.Get"};
            static const char Location_Set[] = {"Location.Set"};
            static const char System_Info[] = {"System.Info"};
            static const char System_TimeSet[] = {"System.TimeSet"};
        }
    }

    class APIRequest {
    public:
        String Prefix;
        String Domain;
        String Address;
        String Command;
        String OptionsString;
        String Data;
        String getOption(unsigned int optionIndex);
        static APIRequest parse(String command);

    protected:
        static String urlDecode(String& str)
        {
            String encodedString = "";
            char c;
            char code0;
            char code1;
            for (int i = 0; i < str.length(); i++){
                c=str.charAt(i);
                if (c == '+') {
                    encodedString += ' ';
                } else if (c == '%') {
                    i++;
                    code0=str.charAt(i);
                    i++;
                    code1=str.charAt(i);
                    c = (hexToInt(code0) << 4) | hexToInt(code1);
                    encodedString += c;
                } else {
                    encodedString += c;
                }
                //yield();
            }
            return encodedString;
        }
        static unsigned char hexToInt(char c)
        {
            if (c >= '0' && c <='9'){
                return((unsigned char)c - '0');
            }
            if (c >= 'a' && c <='f'){
                return((unsigned char)c - 'a' + 10);
            }
            if (c >= 'A' && c <='F'){
                return((unsigned char)c - 'A' + 10);
            }
            return(0);
        }
    };

}}

#endif //HOMEGENIE_MINI_APIREQUEST_H
