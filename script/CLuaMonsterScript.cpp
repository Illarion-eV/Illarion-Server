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


#include "CLuaMonsterScript.hpp"
#include "CMonster.hpp"
#include "CWorld.hpp"
#include "luabind/luabind.hpp"
#include <iostream>
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaMonsterScript::CLuaMonsterScript(std::string filename) throw(ScriptException)
		: CLuaScript(filename) {
}

CLuaMonsterScript::~CLuaMonsterScript() throw() {}

void CLuaMonsterScript::onDeath(CCharacter * Monster) {
	try {
        CWorld::get()->setCurrentScript( this ); 
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("onDeath called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","onDeath called for: " + Monster->name);
		fuse_ptr<CCharacter> fuse_Monster(Monster);
        call("onDeath")( fuse_Monster );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: onDeath called for: " + Monster->name + " " + e.what(),3));
	}
}

bool CLuaMonsterScript::lookAtMonster(CCharacter * source, CCharacter * target, unsigned char mode)
{
    try 
    {
        CWorld::get()->setCurrentScript( this ); 
		fuse_ptr<CCharacter> fuse_source(source);
        fuse_ptr<CCharacter> fuse_target(target);
        call("lookAtMonster")( fuse_source, fuse_target, mode );
        return true;
	} 
    catch (luabind::error &e) 
    {
        writeErrorMsg();
        return false;
	}    
}

void CLuaMonsterScript::receiveText(CCharacter * Monster, CCharacter::talk_type tt, std::string message, CCharacter* cc) {
	try {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("reiceveText called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","receiveText called for: " + Monster->name);
#endif
		fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_cc(cc);
        call("receiveText")( fuse_Monster, (int)tt, message, fuse_cc );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: receiveText called for: " + Monster->name + " " + e.what(),3));       
	}
}

void CLuaMonsterScript::onAttacked(CCharacter * Monster, CCharacter * attacker) {
	try {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("onAttack called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","onAttack called for: " + Monster->name);
#endif        
		fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_attacker(attacker);
        call("onAttacked")( fuse_Monster, fuse_attacker );
	} catch ( luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: onAttack called for: " + Monster->name + " " + e.what(),3));  
              
	}

}

void CLuaMonsterScript::onCasted(CCharacter * Monster, CCharacter * caster) {
	try {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG  
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("onCasted called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","onCasted called for: " + Monster->name);
#endif
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_caster(caster);
        call("onCasted")( fuse_Monster, fuse_caster );        
		
	} catch ( luabind::error &e) {
        writeErrorMsg();
       // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: onCasted called for: " + Monster->name + " " + e.what(),3));
	}
}

void CLuaMonsterScript::useMonster(CCharacter * Monster, CCharacter * user, unsigned short int counter, unsigned short int param, unsigned char ltastate) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("useMonster called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","useMonster called for: " + Monster->name);
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_user(user);
        call("useMonster")( fuse_Monster, fuse_user, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: useMonster called for: " + Monster->name + " " + e.what(),3));   
	}
}

void CLuaMonsterScript::useMonsterWithCharacter(CCharacter * Monster, CCharacter * user, CCharacter * targetChar, unsigned short int counter, unsigned short param, unsigned char ltastate) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("useMonsterWithCharacter called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","useMonsterWithCharacter called for: " + Monster->name);
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_user(user);
        fuse_ptr<CCharacter> fuse_targetChar(targetChar);
        call("useMonsterWithCharacter")( fuse_Monster, fuse_user, fuse_targetChar, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: useMonsterWithCharacter called for: " + Monster->name + " " + e.what(),3)); 
	}
}

void CLuaMonsterScript::useMonsterWithField(CCharacter * Monster, CCharacter * user, position pos, unsigned short int counter, unsigned short param, unsigned char ltastate) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("useMonsterWithField called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","useMonsterWithField called for: " + Monster->name);        
		fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_user(user);
        call("useMonsterWithField")( fuse_Monster, fuse_user, pos, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
       // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: useMonsterWithField called for: " + Monster->name + " " + e.what(),3)); 
	}
}

void CLuaMonsterScript::useMonsterWithItem(CCharacter * Monster, CCharacter * user, ScriptItem item, unsigned short int counter, unsigned short param, unsigned char ltastate) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("useMonsterWithItem called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","useMonsterWithItem called for: " + Monster->name);
		fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_user(user);
        call("useMonsterWithItem")( fuse_Monster, fuse_user, item, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: useMonsterWithItem called for: " + Monster->name + " " + e.what(),3));
	}
}

bool CLuaMonsterScript::actionDisturbed(CCharacter * performer, CCharacter * disturber)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("actionDisturbed called",3));
        CLogger::writeMessage("scripts","actionDisturbed called");
        fuse_ptr<CCharacter> fuse_performer(performer);
        fuse_ptr<CCharacter> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
        return true;
    }
}

void CLuaMonsterScript::onAttack(CCharacter * Monster, CCharacter * target)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("onAttack called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","onAttack called for: " + Monster->name);
#endif
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_target(target);
        call("onAttack")( fuse_Monster, fuse_target );
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();
       // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: onAttack called for: " + Monster->name + " " + e.what(),3));
    }
}

bool CLuaMonsterScript::enemyOnSight(CCharacter * Monster, CCharacter * enemy)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyOnSight called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","enemyOnSight called for: " + Monster->name);
#endif        
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_enemy(enemy);
        return luabind::object_cast<bool>(call("enemyOnSight")( fuse_Monster, fuse_enemy ));
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyOnSight called for: " + Monster->name + " " + e.what(),3));
    }
    return false;
}

bool CLuaMonsterScript::enemyNear(CCharacter * Monster, CCharacter * enemy)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyNear called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","enemyNear called for: " + Monster->name);
#endif         
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        fuse_ptr<CCharacter> fuse_enemy(enemy);
        return luabind::object_cast<bool>(call("enemyNear")( fuse_Monster, fuse_enemy ));    
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
    return false;
}

void CLuaMonsterScript::abortRoute(CCharacter * Monster)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyNear called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","abortRoute called for: " + Monster->name);
#endif         
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        call("abortRoute")( fuse_Monster );    
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

void CLuaMonsterScript::onSpawn(CCharacter * Monster)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
#ifdef MAJORSCRIPTLOG
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("enemyNear called for: " + Monster->name,3));
        CLogger::writeMessage("scripts","onSpawn called for: " + Monster->name);
#endif         
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        call("onSpawn")( fuse_Monster );    
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: enemyNear called for: " + Monster->name + " " + e.what(),3));
    }
}

bool CLuaMonsterScript::setTarget( CCharacter * Monster, std::vector<CPlayer *> & CandidateList, CPlayer * &Target )
{
    Target = 0;
    if( !existsEntrypoint("setTarget") ) return false;
    try
    {
        CWorld::get()->setCurrentScript( this );
#ifdef MAJORSCRIPTLOG
        CLogger::writeMessage("scripts","setTarget called for: " + Monster->name);
#endif
        luabind::object luaCandidateList = luabind::newtable( _luaState );
        int index = 1;
        std::vector<CPlayer *>::iterator it;
        for( it = CandidateList.begin(); it != CandidateList.end(); ++it )
        {
            fuse_ptr<CCharacter> fuse_it(*it);
            luaCandidateList[index++] = fuse_it;
        }
        fuse_ptr<CCharacter> fuse_Monster(Monster);
        index = luabind::object_cast<int>( call("setTarget")( fuse_Monster, luaCandidateList ) ) - 1;
        if( index >= 0 && index < (int)CandidateList.size() )
            Target =  CandidateList[index];
    }
    catch ( luabind::error &e )
    {
        writeErrorMsg();
    }
    return true;
}
