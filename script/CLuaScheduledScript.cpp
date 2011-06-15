#include "luabind/luabind.hpp"
#include "CWorld.hpp"
#include "CLuaScheduledScript.hpp"
#include "CLogger.hpp"


CLuaScheduledScript::CLuaScheduledScript(std::string filename) throw(ScriptException)
    : CLuaScript(filename)
{
}

CLuaScheduledScript::~CLuaScheduledScript() throw() {}

void CLuaScheduledScript::callFunction(std::string name, uint32_t currentCycle, uint32_t lastCycle, uint32_t nextCycle)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaScheduledScript::callFunction called for: " + name, 3));
        CLogger::writeMessage("scripts","CLuaScheduledScript::callFunction called for: " + name);
        call(name)( currentCycle, lastCycle, nextCycle );
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaScheduledScript::callFunction called for: " + name + " " + e.what(),3));
    }
}


