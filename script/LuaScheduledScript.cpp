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


#include "luabind/luabind.hpp"
#include "World.hpp"
#include "LuaScheduledScript.hpp"
#include "Logger.hpp"


LuaScheduledScript::LuaScheduledScript(std::string filename) throw(ScriptException)
    : LuaScript(filename)
{
}

LuaScheduledScript::~LuaScheduledScript() throw() {}

void LuaScheduledScript::callFunction(std::string name, uint32_t currentCycle, uint32_t lastCycle, uint32_t nextCycle)
{
    try
    {
        World::get()->setCurrentScript( this ); 
        //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("LuaScheduledScript::callFunction called for: " + name, 3));
        Logger::writeMessage("scripts","LuaScheduledScript::callFunction called for: " + name);
        call(name)( currentCycle, lastCycle, nextCycle );
    }
    catch (luabind::error &e)
    {
        writeErrorMsg();
         //CWorld::get()->monitoringClientList->sendCommand( new SendMessageTS("Error: LuaScheduledScript::callFunction called for: " + name + " " + e.what(),3));
    }
}


