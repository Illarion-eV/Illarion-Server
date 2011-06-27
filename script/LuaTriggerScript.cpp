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


#include "LuaTriggerScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "Character.hpp"
#include "World.hpp"
#include "fuse_ptr.hpp"

#include "Logger.hpp"

LuaTriggerScript::LuaTriggerScript(std::string filename, position pos) throw(ScriptException)
		: LuaScript(filename), _pos(pos)
{
	init_functions();
}

LuaTriggerScript::~LuaTriggerScript() throw () {}

void LuaTriggerScript::init_functions() {
	luabind::object globals = luabind::globals(_luaState);
	globals["thisField"] = _pos;
}

void LuaTriggerScript::CharacterOnField(Character * character) {
	try 
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::CharacterOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::CharacterOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		fuse_ptr<Character> fuse_Character(character);
        call("CharacterOnField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::CharacterOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
	}
}

void LuaTriggerScript::ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem) {
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::ItemRotsOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::CharacterOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		call("ItemRotsOnField")( oldItem, newItem );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::ItemRotsOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
	}
}

void LuaTriggerScript::MoveFromField(Character * character) {
	try {
        World::get()->setCurrentScript( this ); 
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::MoveFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::MoveFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		fuse_ptr<Character> fuse_Character(character);
        call("MoveFromField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::MoveFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
	}
}

void LuaTriggerScript::MoveToField(Character * character) {
	try 
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::MoveToField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::MoveToField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		fuse_ptr<Character> fuse_Character(character);
        call("MoveToField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::MoveToField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
	}
}

void LuaTriggerScript::PutItemOnField(ScriptItem item, Character * character) {
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::PutItemOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::PutItemOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		fuse_ptr<Character> fuse_Character(character);
        call("PutItemOnField")( item, fuse_Character );
	} catch (luabind::error &e) {
	    writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::PutItemOnField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
        }
}

void LuaTriggerScript::TakeItemFromField(ScriptItem item, Character * character) {
	try {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaTriggerScript::TAkeItemFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z), 3));
        Logger::writeMessage("scripts","LuaTriggerScript::TakeItemFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z));
		fuse_ptr<Character> fuse_Character(character);
        call("TakeItemFromField")( item, fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaTriggerScript::TakeItemFromField called for: " + Logger::toString(_pos.x) + "," + Logger::toString(_pos.y) + "," + Logger::toString(_pos.z) + " " + e.what(),3));
	}
}

