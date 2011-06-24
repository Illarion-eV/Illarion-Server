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


#include "CLuaItemScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "CField.hpp"
#include "CPlayer.hpp"
#include "CWorld.hpp"
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaItemScript::CLuaItemScript(std::string filename, CommonStruct comstr) throw(ScriptException)
		: CLuaScript(filename) , _comstr(comstr) {
	init_functions();
}

CLuaItemScript::~CLuaItemScript() throw() {}

void CLuaItemScript::init_functions() {
	luabind::object globals = luabind::globals(_luaState);
	globals["thisItem"] = _comstr;
}

void CLuaItemScript::UseItem(CCharacter * User, ScriptItem SourceItem, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate ) 
{
	try 
    {
        CWorld::get()->setCurrentScript( this );        
        CLogger::writeMessage("scripts","UseItem called for: " + CLogger::toString(_comstr.id));
		fuse_ptr<CCharacter> fuse_User(User);
        call("UseItem")( fuse_User, SourceItem, TargetItem, counter, param, ltastate );
        
	} catch (luabind::error &e) 
    {
	writeErrorMsg();
	}
}

bool CLuaItemScript::NextCycle()
{
    try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","Nextcycle called for: " + CLogger::toString(_comstr.id));
        call("NextCycle")();
        return true;
    }
    catch ( luabind::error &e)
    {
        writeErrorMsg();
        return false;
    }
    return false;
}

void CLuaItemScript::UseItemWithCharacter(CCharacter * User, ScriptItem SourceItem, CCharacter * character, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","UseItemWithCharacter called for: " + CLogger::toString(_comstr.id));
		fuse_ptr<CCharacter> fuse_User(User);
        fuse_ptr<CCharacter> fuse_character(character);
        call("UseItemWithCharacter")( fuse_User, SourceItem, fuse_character, counter, param, ltastate );
	} catch (luabind::error &e) {
        writeErrorMsg();
	}
}

void CLuaItemScript::UseItemWithField(CCharacter * User, ScriptItem SourceItem, position TargetPos, unsigned short counter, unsigned short param, unsigned char ltastate) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","UseItemWithField called for: " + CLogger::toString(_comstr.id));
		fuse_ptr<CCharacter> fuse_User(User);
        call("UseItemWithField")( fuse_User, SourceItem, TargetPos, counter, param, ltastate );
	} 
    catch (luabind::error &e) 
    {
        writeErrorMsg();
	}
}

bool CLuaItemScript::actionDisturbed(CCharacter * performer, CCharacter * disturber)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","actionDisturbed called for: " + CLogger::toString(_comstr.id));
        fuse_ptr<CCharacter> fuse_performer(performer);
        fuse_ptr<CCharacter> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}

bool CLuaItemScript::LookAtItem(CCharacter * who, ScriptItem t_item) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","LookAtItem called for: " + CLogger::toString(_comstr.id));
    	fuse_ptr<CCharacter> fuse_who(who);
        call("LookAtItem")( fuse_who, t_item );
		return true;
	} 
    catch (luabind::error &e) 
    {
        writeErrorMsg();
        return false;
	}
	return false;
}

bool CLuaItemScript::MoveItemBeforeMove(CCharacter * who, ScriptItem sourceItem,ScriptItem targetItem) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","MoveItemBeforeMove called for: " + CLogger::toString(_comstr.id));        
        fuse_ptr<CCharacter> fuse_who(who);
		return luabind::object_cast<bool>(call("MoveItemBeforeMove")( fuse_who, sourceItem, targetItem ));
	} catch (luabind::error &e) {
        writeErrorMsg();
	}
	return true;
}

void CLuaItemScript::MoveItemAfterMove(CCharacter * who, ScriptItem sourceItem, ScriptItem targetItem) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","MoveItemAfterMove called for: " + CLogger::toString(_comstr.id));
		fuse_ptr<CCharacter> fuse_who(who);
        call("MoveItemAfterMove")( fuse_who, sourceItem, targetItem );
	} 
    catch (luabind::error &e) 
    {
        writeErrorMsg();        
	}
}

void CLuaItemScript::CharacterOnField(CCharacter * who) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        CLogger::writeMessage("scripts","CharacterOnField called for: " + CLogger::toString(_comstr.id));
		fuse_ptr<CCharacter> fuse_who(who);
        call("CharacterOnField")( fuse_who );
	} catch (luabind::error &e) {
        writeErrorMsg(); 
	}
}


