#include "CLuaLoginScript.hpp"
#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaLoginScript::CLuaLoginScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaLoginScript::~CLuaLoginScript() throw() {}

bool CLuaLoginScript::onLogin(CCharacter * cc)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        fuse_ptr<CCharacter> fuse_cc(cc);
        call("onLogin")( fuse_cc );
        return true;
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();    
    }
    return false;
}
