#include "CLuaReloadScript.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"

CLuaReloadScript::CLuaReloadScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaReloadScript::~CLuaReloadScript() throw() {}

bool CLuaReloadScript::onReload()
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        call("onReload")();
        return true;
    }
    catch (luabind::error &e)
    {
         std::cerr << "ERROR: " << lua_tostring( _luaState, -1 ) << std::endl;
         writeErrorMsg();
         return false;    
    }
    return false;
}
