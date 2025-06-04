/*
 * HomeGenie-Mini (c) 2018-2025 G-Labs
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

#ifndef HOMEGENIE_MINI_COMMONAPI_H
#define HOMEGENIE_MINI_COMMONAPI_H

#define UI_WIDGETS_FIELD_TYPE_NUMBER "number"
#define UI_WIDGETS_FIELD_TYPE_SLIDER "slider"
#define UI_WIDGETS_FIELD_TYPE_BUTTON "button"
#define UI_WIDGETS_FIELD_TYPE_SELECT "select"
#define UI_WIDGETS_FIELD_TYPE_CHECKBOX "checkbox"
#define UI_WIDGETS_FIELD_TYPE_SELECT_FILTER "select.filter"
#define UI_WIDGETS_FIELD_TYPE_MODULE_TEXT "module.text"

namespace Service { namespace API {

    namespace ControlApi {
        // generic switch / light
        static const char Control_On[] = "Control.On";
        static const char Control_Off[] = "Control.Off";
        static const char Control_Toggle[] = "Control.Toggle";
        static const char Control_Level[] = "Control.Level";
        static const char Control_ColorHsb[] = "Control.ColorHsb";
        // shutters/ motors / door locks
        static const char Control_Open[] = "Control.Open";
        static const char Control_Close[] = "Control.Close";
    }

    namespace AutomationApi {
        static const char Programs_Enable[] = "Programs.Enable";
        static const char Programs_Disable[] = "Programs.Disable";
        static const char Scheduling_Add[] = "Scheduling.Add";
        static const char Scheduling_Update[] = "Scheduling.Update";
        static const char Scheduling_Get[] = "Scheduling.Get";
        static const char Scheduling_ModuleUpdate[] = "Scheduling.ModuleUpdate";
        static const char Scheduling_ListOccurrences[] = "Scheduling.ListOccurrences";
        static const char Scheduling_Enable[] = "Scheduling.Enable";
        static const char Scheduling_Disable[] = "Scheduling.Disable";
        static const char Scheduling_Delete[] = "Scheduling.Delete";
        static const char Scheduling_List[] = "Scheduling.List";
        static const char Scheduling_Templates[] = "Scheduling.Templates";
    }

    namespace ConfigApi {
        static const char Modules_List[] = "Modules.List";
        static const char Modules_Get[] = "Modules.Get";
        static const char Modules_UpdateInfo[] = "Modules.UpdateInfo";
        static const char Modules_ParameterGet[] = "Modules.ParameterGet";
        static const char Modules_ParameterSet[] = "Modules.ParameterSet";
        static const char Modules_StatisticsGet[] = "Modules.StatisticsGet";
        static const char Groups_List[] = "Groups.List";
        static const char WebSocket_GetToken[] = "WebSocket.GetToken";
        static const char System_Configure[] = "System.Configure";
        namespace SystemApi {
            static const char Location_Get[] = "Location.Get";
            static const char Location_Set[] = "Location.Set";
            static const char System_Info[] = "System.Info";
            static const char System_DataSet[] = "System.DataSet";
            static const char System_TimeSet[] = "System.TimeSet";
        }
    }

    namespace WidgetApi {
        static const char DisplayModule[] = "Widget.DisplayModule";
        namespace Implements {
            static const char Scheduling[] = "Widget.Implements.Scheduling";
            static const char Scheduling_ModuleEvents[] = "Widget.Implements.Scheduling.ModuleEvents";
        }
        namespace Preference {
            static const char AudioLight[] = "Widget.Preference.AudioLight";
        }
    }

    namespace ModuleApi {
        namespace Property {
            static const char SystemInfo[] = "System.Info";
        }
        namespace ModuleType {
            static const char Generic[] = "Generic";
            static const char Switch[] = "Switch";
            static const char Light[] = "Light";
            static const char Dimmer[] = "Dimmer";
            static const char Color[] = "Color";
            static const char Sensor[] = "Sensor";
            static const char Motor[] = "Motor";
        }
    }

    namespace CameraApi {
        static const char Camera_GetPicture[] = "Camera.GetPicture";
        namespace Property {
            static const char RemoteCamera_EndPoint[] = "RemoteCamera.EndPoint";
        }
    }

    namespace DisplayApi {
        namespace Configuration {
            namespace Display {
                static const char Brightness[] = "disp-bri";
            }
            namespace ScreenSaver {
                static const char TimeoutSeconds[] = "dsbrd-ssts";
            }
        }
        namespace Options {
            static const char Display_ScreenSaverTimeout[] = "Display.ScreenSaverTimeout";
            static const char Display_Brightness[] = "Display.Brightness";
        }
    }
}}

#endif //HOMEGENIE_MINI_COMMONAPI_H
