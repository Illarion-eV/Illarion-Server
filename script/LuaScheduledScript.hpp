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


#ifndef C_LUA_SCHEDULED_SCRIPT
#define C_LUA_SCHEDULED_SCRIPT

#include "LuaScript.hpp"

/** class definition so pointers to the class can be used **/
class World;

class LuaScheduledScript : public LuaScript {
public:

    /**
    *standard constructor which loads the script and adds basic functions
    *@see LuaScript
    *@param filename the name of the script which should be loaded
    *@param world a pointer to the current gameworld
    */
    LuaScheduledScript(std::string filename) throw(ScriptException);

    /**
    *virtual destructor
    */
    virtual ~LuaScheduledScript() throw();

    /**
    *a function which calls a special function  inside the script
    *@param name the name of the function which should be called (the current cycle is given to that lua function)
    *@param currentCycle the current cycle
    *@param lastCycle the last Cycle when the script was called
    *@param nextCycle the next Cycle when the script is called
    */
    void callFunction(std::string name, uint32_t currentCycle, uint32_t lastCycle, uint32_t nextCycle);

private:

    //bool m_dataOk; /**< if true all loading was successfully otherwise false */
    LuaScheduledScript(const LuaScheduledScript &);
    LuaScheduledScript &operator=(LuaScheduledScript &);
};
#endif
