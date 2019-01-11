//
// Created by gene on 10/01/19.
//

#include "ProgramEngine.h"
#include "io/Logger.h"

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
