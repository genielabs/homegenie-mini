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
 * - 2019-01-13 Initial release
 *
 */

#ifndef HOMEGENIE_MINI_PROGRAMENGINE_H
#define HOMEGENIE_MINI_PROGRAMENGINE_H
/*
#include "tinyjs/TinyJS.h"
#include "tinyjs/TinyJS_Functions.h"
#include "tinyjs/TinyJS_MathFunctions.h"
*/
#include <io/Logger.h>

namespace Scripting {

    class ProgramEngine {
    public:
        ProgramEngine();
        void begin();
        void test();
    };

}

#endif //HOMEGENIE_MINI_PROGRAMENGINE_H
