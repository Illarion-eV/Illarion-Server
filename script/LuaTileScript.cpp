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


#include "LuaTileScript.hpp"
#include "World.hpp"
#include "Character.hpp"
#include "luabind/luabind.hpp"
#include "Logger.hpp"
#include "fuse_ptr.hpp"

LuaTileScript::LuaTileScript(std::string filename, TilesStruct tile) throw (ScriptException)
		: LuaScript(filename), thisTile(tile) {
	init_functions();
}

LuaTileScript::~LuaTileScript() throw () {}

void LuaTileScript::init_functions() {
	luabind::object globals = luabind::globals(_luaState);
	globals["thisTile"] = thisTile;
}

void LuaTileScript::useTile(Character * user, position pos, unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTileScript::useTile called for: " + thisTile.English, 3));
        Logger::writeMessage("scripts","LuaTileScript::useTile called for: " + thisTile.English);
		fuse_ptr<Character> fuse_user(user);
        call("useTile")( fuse_user, pos, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
        // World::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTileScript::useTile called for: " + thisTile.English + " " + e.what(),3));
	}
}

void LuaTileScript::useTileWithCharacter(Character * user, position pos, Character * character,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	std::cout<<"Try to call useTileWithCharacter!"<<std::endl;
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTileScript::useTileWithCharacter called for: " + thisTile.English, 3));
        Logger::writeMessage("scripts","LuaTileScript::useTileWithCharacter called for: " + thisTile.English);
		fuse_ptr<Character> fuse_user(user);
        fuse_ptr<Character> fuse_character(character);
        call("useTileWithCharacter")( fuse_user, pos, fuse_character, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTileScript::useTileWitCharacter called for: " + thisTile.English + " " + e.what(),3));
	}
}

void LuaTileScript::useTileWithField(Character * user, position pos, position posnew,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try 
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTileScript::useTileWithField called for: " + thisTile.English, 3));
        Logger::writeMessage("scripts","LuaTileScript::useTileWithField called for: " + thisTile.English);
		fuse_ptr<Character> fuse_user(user);
        call("useTileWithField")( fuse_user, pos, posnew, counter, param, ltastate );
	} 
    catch (luabind::error &e) 
    {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTileScript::useTileWithField called for: " + thisTile.English + " " + e.what(),3));

	}
}

void LuaTileScript::useTileWithItem(Character * user, position pos, ScriptItem item,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTileScript::useTileWithItem called for: " + thisTile.English, 3));
        Logger::writeMessage("scripts","LuaTileScript::useTileWithItem called for: " + thisTile.English);
		fuse_ptr<Character> fuse_user(user);
        call("useTileWithItem")( fuse_user, pos, item, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTileScript::useTileWithItem called for: " + thisTile.English + " " + e.what(),3));
	}
}

bool LuaTileScript::actionDisturbed(Character * performer, Character * disturber)
{
    try
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTileScript::actionDisturbed called for: " + thisTile.English, 3));
        Logger::writeMessage("scripts","LuaTileScript::actionDisturbed called for: " + thisTile.English);
        fuse_ptr<Character> fuse_performer(performer);
        fuse_ptr<Character> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}

