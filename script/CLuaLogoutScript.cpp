#include "CLuaLogoutScript.hpp"
#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaLogoutScript::CLuaLogoutScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaLogoutScript::~CLuaLogoutScript() throw() {}

bool CLuaLogoutScript::onLogout(CCharacter * cc)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        fuse_ptr<CCharacter> fuse_cc(cc);
        call("onLogout")( fuse_cc );
        return true;
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();    
    }
    return false;
}
