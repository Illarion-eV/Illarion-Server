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


#include "LuaMagicScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "World.hpp"
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaMagicScript::LuaMagicScript(std::string filename, unsigned long int MagicFlag) throw(ScriptException)
		: LuaScript(filename), _MagicFlag(MagicFlag) {
	init_functions();
}

LuaMagicScript::~LuaMagicScript() throw() {}

void LuaMagicScript::init_functions()
{
	luabind::object globals = luabind::globals(_luaState);
	globals["thisSpell"] = _MagicFlag;
}

void LuaMagicScript::CastMagic(Character * caster, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try 
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("CastMagic called for: " + Logger::toString(_MagicFlag ),3));
        Logger::writeMessage("scripts","CastMagic called for: " + Logger::toString(_MagicFlag));
		fuse_ptr<Character> fuse_caster(caster);
        call("CastMagic")( fuse_caster, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: CastMagic called for: " + Logger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void LuaMagicScript::CastMagicOnCharacter(Character * caster, Character * target, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try 
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("CastMagicOnCharacter called for: " + Logger::toString(_MagicFlag ),3));
        Logger::writeMessage("scripts","CastMagicOnCharacter called for: " + Logger::toString(_MagicFlag));        
		fuse_ptr<Character> fuse_caster(caster);
        fuse_ptr<Character> fuse_target(target);
        call("CastMagicOnCharacter")( fuse_caster, fuse_target, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: CastMagicOnCharacter called for: " + Logger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void LuaMagicScript::CastMagicOnField(Character * caster, position pos, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try {
        World::get()->setCurrentScript( this ); 
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("CastMagicOnField called for: " + Logger::toString(_MagicFlag ),3));
        Logger::writeMessage("scripts","CastMagicOnField called for: " + Logger::toString(_MagicFlag)); 
		fuse_ptr<Character> fuse_caster(caster);
        call("CastMagicOnField")( fuse_caster, pos, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: CastMagicOnField called for: " + Logger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

void LuaMagicScript::CastMagicOnItem(Character * caster, ScriptItem TargetItem, unsigned short counter, unsigned short int param, unsigned char ltastate ) {
	// ToDo Script Calling
	try {
        World::get()->setCurrentScript( this ); 
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("CastMagicOnItem called for: " + Logger::toString(_MagicFlag ),3));
        Logger::writeMessage("scripts","CastMagicOnItem called for: " + Logger::toString(_MagicFlag)); 
		fuse_ptr<Character> fuse_caster(caster);
        call("CastMagicOnItem")( fuse_caster, TargetItem, counter, param, ltastate  );
	} catch (luabind::error &e) {
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: CastMagicOnItem called for: " + Logger::toString(_MagicFlag ) + " " + e.what(),3));
        writeErrorMsg();
	}
}

bool LuaMagicScript::actionDisturbed(Character * performer, Character * disturber)
{
    try
    {
        World::get()->setCurrentScript( this ); 
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("actionDisturbed called for: " + Logger::toString(_MagicFlag ),3));
        Logger::writeMessage("scripts","actionDisturbed called for: " + Logger::toString(_MagicFlag)); 
        fuse_ptr<Character> fuse_performer(performer);
        fuse_ptr<Character> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}
