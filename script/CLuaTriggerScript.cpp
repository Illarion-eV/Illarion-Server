#include "CLuaTriggerScript.hpp"

#include "luabind/luabind.hpp"
#include <iostream>

#include "CCharacter.hpp"
#include "CWorld.hpp"
#include "fuse_ptr.hpp"

#include "CLogger.hpp"

CLuaTriggerScript::CLuaTriggerScript(std::string filename, position pos) throw(ScriptException)
		: CLuaScript(filename), _pos(pos)
{
	init_functions();
}

CLuaTriggerScript::~CLuaTriggerScript() throw () {}

void CLuaTriggerScript::init_functions() {
	luabind::object globals = luabind::globals(_luaState);
	globals["thisField"] = _pos;
}

void CLuaTriggerScript::CharacterOnField(CCharacter * Character) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::CharacterOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::CharacterOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		fuse_ptr<CCharacter> fuse_Character(Character);
        call("CharacterOnField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::CharacterOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
	}
}

void CLuaTriggerScript::ItemRotsOnField(ScriptItem oldItem, ScriptItem newItem) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::ItemRotsOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::CharacterOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		call("ItemRotsOnField")( oldItem, newItem );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::ItemRotsOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
	}
}

void CLuaTriggerScript::MoveFromField(CCharacter * Character) {
	try {
        CWorld::get()->setCurrentScript( this ); 
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::MoveFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::MoveFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		fuse_ptr<CCharacter> fuse_Character(Character);
        call("MoveFromField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::MoveFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
	}
}

void CLuaTriggerScript::MoveToField(CCharacter * Character) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::MoveToField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::MoveToField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		fuse_ptr<CCharacter> fuse_Character(Character);
        call("MoveToField")( fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::MoveToField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
	}
}

void CLuaTriggerScript::PutItemOnField(ScriptItem item, CCharacter * Character) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::PutItemOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::PutItemOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		fuse_ptr<CCharacter> fuse_Character(Character);
        call("PutItemOnField")( item, fuse_Character );
	} catch (luabind::error &e) {
	    writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::PutItemOnField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
        }
}

void CLuaTriggerScript::TakeItemFromField(ScriptItem item, CCharacter * Character) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTriggerScript::TAkeItemFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z), 3));
        CLogger::writeMessage("scripts","CLuaTriggerScript::TakeItemFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z));
		fuse_ptr<CCharacter> fuse_Character(Character);
        call("TakeItemFromField")( item, fuse_Character );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTriggerScript::TakeItemFromField called for: " + CLogger::toString(_pos.x) + "," + CLogger::toString(_pos.y) + "," + CLogger::toString(_pos.z) + " " + e.what(),3));
	}
}

