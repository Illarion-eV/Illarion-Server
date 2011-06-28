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


#include "World.hpp"
#include "Command.hpp"
#include <sstream>
#include <regex.h>
#include <list>
#include "data/TilesModificatorTable.hpp"
#include "data/TilesTable.hpp"
#include "data/ArmorObjectTable.hpp"
#include "data/WeaponObjectTable.hpp"
#include "data/ContainerObjectTable.hpp"
#include "data/CommonObjectTable.hpp"
#include "data/NamesObjectTable.hpp"
#include "data/MonsterTable.hpp"
#include "data/SpellTable.hpp"
#include "data/TriggerTable.hpp"
#include "data/MonsterAttackTable.hpp"
#include "data/NaturalArmorTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
//We need this for the standard Fighting Script.
#include "script/LuaWeaponScript.hpp"
#include <boost/shared_ptr.hpp>
#include "Logger.hpp"
#include "Player.hpp"

#include <iostream>

extern std::ofstream talkfile;

void set_spawn_command( World*, Player*, const std::string& );
void import_maps_command( World*, Player*, const std::string& );
void create_area_command( World*, Player*, const std::string& );
void set_login( World*, Player*, const std::string& );


void World::montool_kill_command(Player * c)
{
    if ( !c->hasGMRight(gmr_reload) )return;
    MONSTERVECTOR::iterator mIterator;
    uint32_t counter = 0;
    for ( mIterator = Monsters.begin() ; mIterator != Monsters.end(); ++mIterator)
    {
        //Kill the monster we have found
        (*mIterator)->increaseAttrib("hitpoints",-((*mIterator)->increaseAttrib("hitpoints",0)+1) );
        ++counter;
    }
    //monitoringClientList->sendCommand( new SendMessageTS("Sucessfully killed: "+Logger::toString(counter)+" monsters per nuke from: "+c->name+"("+Logger::toString(c->id)+")",0));
}

void World::montool_reload_command( Player * c)
{
    /*
    if ( c->hasGMRight(gmr_reload) )
    {
        if (reload_tables(c) ) 
            //monitoringClientList->sendCommand( new SendMessageTS("db tables loaded succesfully! from "+c->name+"("+Logger::toString(c->id)+")",0));
        else
            //monitoringClientList->sendCommand( new SendMessageTS("error while loading tables! from "+c->name+"("+Logger::toString(c->id)+")",0));
    }
    */
}

void World::montool_kickall_command( Player * c)
{
    if ( c->hasGMRight( gmr_forcelogout ) ) 
    {
	    forceLogoutOfAllPlayers();
        //monitoringClientList->sendCommand( new SendMessageTS("all players forced to logout! from "+c->name+"("+Logger::toString(c->id)+")",0));
    }
}

void World::montool_import_maps_command( Player * c)
{
    if ( !c->hasGMRight(gmr_import) )return;
    {
	    load_maps();
        //monitoringClientList->sendCommand( new SendMessageTS("maps imported! from "+c->name+"("+Logger::toString(c->id)+")",0));
    }
}

void World::montool_set_login(Player * c, const std::string& st) 
{
    if ( !c->hasGMRight(gmr_loginstate) )return;
	configOptions["disable_login"] = st;
	//std::cout << "nologin set to " << st << std::endl;
	//monitoringClientList->sendCommand( new SendMessageTS("set login set to: " + st +"! from "+c->name+"("+Logger::toString(c->id)+")",0));
    
}
