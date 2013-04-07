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
#include <sstream>
#include <list>
#include "data/MonsterTable.hpp"
#include "data/ScheduledScriptsTable.hpp"
//We need this for the standard Fighting Script.
#include "script/LuaWeaponScript.hpp"
#include <boost/shared_ptr.hpp>
#include "Logger.hpp"
#include "Player.hpp"
#include "Monster.hpp"

#include <iostream>

void World::montool_kill_command(Player *c) {
    if (!c->hasGMRight(gmr_reload)) {
        return;
    }

    uint32_t counter = 0;

    Monsters.for_each([&counter](Monster *monster) {
        monster->increaseAttrib("hitpoints", -(monster->increaseAttrib("hitpoints",0)+1));
        ++counter;
    });

    //monitoringClientList->sendCommand( new SendMessageTS("Sucessfully killed: "+Logger::toString(counter)+" monsters per nuke from: "+c->to_string()+"("+Logger::toString(c->id)+")",0));
}

void World::montool_reload_command(Player *c) {
    /*
    if ( c->hasGMRight(gmr_reload) )
    {
        if (reload_tables(c) )
            //monitoringClientList->sendCommand( new SendMessageTS("db tables loaded succesfully! from "+c->to_string()+"("+Logger::toString(c->id)+")",0));
        else
            //monitoringClientList->sendCommand( new SendMessageTS("error while loading tables! from "+c->to_string()+"("+Logger::toString(c->id)+")",0));
    }
    */
}

void World::montool_kickall_command(Player *c) {
    if (c->hasGMRight(gmr_forcelogout)) {
        forceLogoutOfAllPlayers();
        //monitoringClientList->sendCommand( new SendMessageTS("all players forced to logout! from "+c->to_string()+"("+Logger::toString(c->id)+")",0));
    }
}

void World::montool_set_login(Player *c, const std::string &st) {
    if (!c->hasGMRight(gmr_loginstate)) {
        return;
    }

    bool enable = true;

    if (st == "true") {
        enable = false;
    }

    World::get()->allowLogin(enable);
    Logger::info(LogFacility::Admin) << c->to_string() << " set allowLogin to " << enable << Log::end;
    //monitoringClientList->sendCommand( new SendMessageTS("set login set to: " + st +"! from "+c->to_string()+"("+Logger::toString(c->id)+")",0));

}
