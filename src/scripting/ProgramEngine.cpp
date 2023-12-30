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

#include "ProgramEngine.h"

namespace Scripting {

    using namespace IO;

    ProgramEngine::ProgramEngine() {

    }

    void ProgramEngine::begin() {

    }

    void ProgramEngine::test() {
/*
        CTinyJS s;
        registerFunctions(&s);
        registerMathFunctions(&s);
        s.root->addChild("result", new CScriptVar("0",SCRIPTVAR_INTEGER));
        try {
            s.execute("var abd = '123'; return false;");
        } catch (CScriptException *e) {
            Logger::error("ERROR: %s\n", e->text.c_str());
        }
        bool pass = s.root->getParameter("result")->getBool();
        Logger::info("PASS = %s", pass ? "OK" : "ERROR");
*/
    }

}
