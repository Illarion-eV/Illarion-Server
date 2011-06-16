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


#include "CLuaTileScript.hpp"
#include "CWorld.hpp"
#include "CCharacter.hpp"
#include "luabind/luabind.hpp"
#include "CLogger.hpp"
#include "fuse_ptr.hpp"

CLuaTileScript::CLuaTileScript(std::string filename, TilesStruct tile) throw (ScriptException)
		: CLuaScript(filename), thisTile(tile) {
	init_functions();
}

CLuaTileScript::~CLuaTileScript() throw () {}

void CLuaTileScript::init_functions() {
	luabind::object globals = luabind::globals(_luaState);
	globals["thisTile"] = thisTile;
}

void CLuaTileScript::useTile(CCharacter * user, position pos, unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTileScript::useTile called for: " + thisTile.English, 3));
        CLogger::writeMessage("scripts","CLuaTileScript::useTile called for: " + thisTile.English);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useTile")( fuse_user, pos, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
        // CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTileScript::useTile called for: " + thisTile.English + " " + e.what(),3));
	}
}

void CLuaTileScript::useTileWithCharacter(CCharacter * user, position pos, CCharacter * character,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	std::cout<<"Try to call useTileWithCharacter!"<<std::endl;
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTileScript::useTileWithCharacter called for: " + thisTile.English, 3));
        CLogger::writeMessage("scripts","CLuaTileScript::useTileWithCharacter called for: " + thisTile.English);
		fuse_ptr<CCharacter> fuse_user(user);
        fuse_ptr<CCharacter> fuse_character(character);
        call("useTileWithCharacter")( fuse_user, pos, fuse_character, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTileScript::useTileWitCharacter called for: " + thisTile.English + " " + e.what(),3));
	}
}

void CLuaTileScript::useTileWithField(CCharacter * user, position pos, position posnew,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try 
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTileScript::useTileWithField called for: " + thisTile.English, 3));
        CLogger::writeMessage("scripts","CLuaTileScript::useTileWithField called for: " + thisTile.English);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useTileWithField")( fuse_user, pos, posnew, counter, param, ltastate );
	} 
    catch (luabind::error &e) 
    {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTileScript::useTileWithField called for: " + thisTile.English + " " + e.what(),3));

	}
}

void CLuaTileScript::useTileWithItem(CCharacter * user, position pos, ScriptItem item,  unsigned short int counter, unsigned int param, unsigned char ltastate) {
	try {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTileScript::useTileWithItem called for: " + thisTile.English, 3));
        CLogger::writeMessage("scripts","CLuaTileScript::useTileWithItem called for: " + thisTile.English);
		fuse_ptr<CCharacter> fuse_user(user);
        call("useTileWithItem")( fuse_user, pos, item, counter, param, ltastate );
	} catch (luabind::error &e) {
         writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("Error: CLuaTileScript::useTileWithItem called for: " + thisTile.English + " " + e.what(),3));
	}
}

bool CLuaTileScript::actionDisturbed(CCharacter * performer, CCharacter * disturber)
{
    try
    {
        CWorld::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new CSendMessageTS("CLuaTileScript::actionDisturbed called for: " + thisTile.English, 3));
        CLogger::writeMessage("scripts","CLuaTileScript::actionDisturbed called for: " + thisTile.English);
        fuse_ptr<CCharacter> fuse_performer(performer);
        fuse_ptr<CCharacter> fuse_disturber(disturber);
        return luabind::object_cast<bool>(call("actionDisturbed")( fuse_performer, fuse_disturber ));
    }
    catch ( luabind::error &e)
    {
        return true;
    }
}

