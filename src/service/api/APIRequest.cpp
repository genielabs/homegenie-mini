/*
 * HomeGenie-Mini (c) 2018-2019 G-Labs
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
 * - 2019-13-01 Initial release
 *
 */

#include "APIRequest.h"

namespace Service { namespace API {

    String APIRequest::getOption(unsigned int optionIndex) {
        // TODO: ...
        unsigned int o = 0, ci = 0;
        int oi = OptionsString.indexOf('/');
        if (oi < 0) return OptionsString;
        String option;
        do {
            option = OptionsString.substring(ci, oi-1);
            ci = oi+1;
            o++;
        } while (o < optionIndex);
        return option;
    }

    /// Parse an API command request URL
    /// \param command API URL string
    /// \return the parsed ApiCommand instance.
    APIRequest APIRequest::parse(String command) {
        auto apiCommand = APIRequest();
        int argIndex = 0;
        int i = command.indexOf('/', 1);
        while (i > 0) {
            auto arg = command.substring(0, i);
            command = command.substring(i+1);
            switch (argIndex++) {
                // case 0: is the "/api" prefix
                case 0:
                    if (arg.startsWith("/")) {
                        arg = arg.substring(1);
                    }
                    apiCommand.Prefix = arg;
                    break;
                case 1:
                    apiCommand.Domain = arg;
                    IO::Logger::trace("%s parse() >> [Domain = '%s']", APIREQUEST_LOG_PREFIX, arg.c_str());
                    break;
                case 2:
                    apiCommand.Address = arg;
                    IO::Logger::trace("%s parse() >> [Address = '%s']", APIREQUEST_LOG_PREFIX, arg.c_str());
                    break;
                case 3:
                    apiCommand.Command = arg;
                    apiCommand.OptionsString = command;
                    IO::Logger::trace("%s parse() >> [Command = '%s']", APIREQUEST_LOG_PREFIX, arg.c_str());
                    IO::Logger::trace("%s parse() >> [Options = '%s']", APIREQUEST_LOG_PREFIX, command.c_str());
                    return apiCommand;
            }
            i = command.indexOf('/');
            if (i < 0 && argIndex == 3) {
                apiCommand.Command = command;
                IO::Logger::trace("%s parse() >> [Command = '%s']", APIREQUEST_LOG_PREFIX, command.c_str());
            }
        }
        return apiCommand;
    }

}}
