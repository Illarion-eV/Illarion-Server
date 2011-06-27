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


#include "LuaNPCScript.hpp"
#include "NPC.hpp"
#include "World.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "Logger.hpp"
#include "fuse_ptr.hpp"

CLuaNPCScript::CLuaNPCScript(std::string filename, CNPC* thisnpc) throw(ScriptException)
		: CLuaScript(filename), _thisnpc(thisnpc)
{
	init_functions();
}

CLuaNPCScript::~CLuaNPCScript() throw() {}

void CLuaNPCScript::init_functions() 
{
	luabind::object globals = luabind::globals(_luaState);
    fuse_ptr<CCharacter> fuse__thisnpc(_thisnpc);
	globals["thisNPC"] = fuse__thisnpc;
}

void CLuaNPCScript::nextCycle() {
	init_functions();
    try {
        CWorld::get()->setCurrentScript( this ); 
		call("nextCycle")();
	} catch (luabind::error &e) {
        writeErrorMsg();
	}
}

bool CLuaNPCScript::lookAtNpc(CCharacter * source, unsigned char mode)
{
    init_functions();
    try 
    {
        CWorld::get()->setCurrentScript( this ); 
		fuse_ptr<CCharacter> fuse_source(source);
        call("lookAtNpc")( fuse_source, mode );
        return true;
	} 
    catch (luabind::error &e) 
    {
        writeErrorMsg();
        return false;      
	}    
}

// we heard <cc> say <message>
void CLuaNPCScript::receiveText(CCharacter::talk_type tt, std::string message, CCharacter* cc) {
	init_functions();
    try {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::receiveText called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::receiveText called for: " + _thisnpc->name);
#endif        
        fuse_ptr<CCharacter> fuse_cc(cc);
		call("receiveText")( (int)tt, message, fuse_cc );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaNPCScript::receiveText called for: " + _thisnpc->name + " " + e.what(),3));
	}
}

void CLuaNPCScript::useNPC(CCharacter * user, unsigned short counter, unsigned short int param, unsigned char ltastate) {
	init_functions();
    try {
        CWorld::get()->setCurrentScript( this ); 
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::useNPC called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::useNPC called for: " + _thisnpc->name);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useNPC")( fuse_user, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaNPCScript::useNPC called for: " + _thisnpc->name + " " + e.what(),3));
	}
}

void CLuaNPCScript::useNPCWithCharacter(CCharacter * user, CCharacter * targetChar, unsigned short counter, unsigned short int param, unsigned char ltastate) {
	init_functions();
    try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name);
		fuse_ptr<CCharacter> fuse_user(user);
        fuse_ptr<CCharacter> fuse_targetChar(targetChar);
        call("useNPCWithCharacter")( fuse_user, fuse_targetChar, counter, param, ltastate  );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaNPCScript::useNPCWithCharacter called for: " + _thisnpc->name + " " + e.what(),3));
	}
}

void CLuaNPCScript::useNPCWithField(CCharacter * user, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate) {
	init_functions();
    try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::useNPCWithField called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::useNPCWithField called for: " + _thisnpc->name);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useNPCWithField")( fuse_user, pos, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaNPCScript::useNPCWithField called for: " + _thisnpc->name + " " + e.what(),3));
	}
}

void CLuaNPCScript::useNPCWithItem(CCharacter * user, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate) {
	init_functions();
    try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::useNPCWithItem called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::useNPCWithItem called for: " + _thisnpc->name);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useNPCWithItem")( fuse_user, TargetItem, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaNPCScript::useNPCWithItem called for: " + _thisnpc->name + " " + e.what(),3));
	}
}

bool CLuaNPCScript::actionDisturbed(CCharacter * performer, CCharacter * disturber)
{
    init_functions();
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaNPCScript::actionDisturbed called for: " + _thisnpc->name,3));
        CLogger::writeMessage("scripts","CLuaNPCScript::actionDisturbed called for: " + _thisnpc->name);
        fuse_ptr<CCharacter> fuse_performer(performer);
        fuse_ptr<CCharacter> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}

void CLuaNPCScript::characterOnSight(CCharacter * npc, CCharacter * enemy)
{
    init_functions();
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyOnSight called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","characterOnSight called for: " + npc->name);
#endif        
        fuse_ptr<CCharacter> fuse_npc(npc);
        fuse_ptr<CCharacter> fuse_enemy(enemy);
        call("characterOnSight")( fuse_npc, fuse_enemy );
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyOnSight called for: " + Monster->name + " " + e.what(),3));
    }
}

void CLuaNPCScript::characterNear(CCharacter * npc, CCharacter * enemy)
{
    init_functions();
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyNear called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","characterNear called for: " + npc->name);
#endif         
        fuse_ptr<CCharacter> fuse_npc(npc);
        fuse_ptr<CCharacter> fuse_enemy(enemy);
        call("characterNear")( fuse_npc, fuse_enemy );    
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void CLuaNPCScript::abortRoute(CCharacter * npc)
{
    init_functions();
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyNear called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","abortRoute called for: " + npc->name);
#endif         
        fuse_ptr<CCharacter> fuse_npc(npc);
        call("abortRoute")( fuse_npc );    
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void CLuaNPCScript::beforeReload()
{
    init_functions();
    try
    {
        CWorld::get()->setCurrentScript( this );
        CLogger::writeMessage("scripts","CLuaNPCScript::beforeReload called for: " + _thisnpc->name);
	call("beforeReload")();
    } catch (luabind::error &e) {
        writeErrorMsg();
    }
}
