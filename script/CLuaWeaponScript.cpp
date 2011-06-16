//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


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
