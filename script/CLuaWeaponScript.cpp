#include "CLuaWeaponScript.hpp"
#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaWeaponScript::CLuaWeaponScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename)
{
}

CLuaWeaponScript::~CLuaWeaponScript() throw() {}

bool CLuaWeaponScript::onAttack(CCharacter * Attacker, CCharacter * Defender, unsigned char pos)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        fuse_ptr<CCharacter> fuse_Attacker(Attacker);
        fuse_ptr<CCharacter> fuse_Defender(Defender);
        call("onAttack")( fuse_Attacker, fuse_Defender, pos );
        return true;
    }
    catch (luabind::error &e)
    {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaWeaponScript::onAttack called for: " + Attacker->name + " " + Defender->name + + e.what(),3));
	     return false;    
    }
    return false;
}

bool CLuaWeaponScript::onDefend(CCharacter * Attacker, CCharacter * Defender)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        fuse_ptr<CCharacter> fuse_Attacker(Attacker);
        fuse_ptr<CCharacter> fuse_Defender(Defender);
        call("onDefend")( fuse_Attacker, fuse_Defender );
        return true;
    }
    catch (luabind::error &e)
    {
        return false;    
    }
    return false;
}
