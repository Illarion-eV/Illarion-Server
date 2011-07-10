/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCHEDULED_SCRIPTS_TABLE_HPP
#define SCHEDULED_SCRIPTS_TABLE_HPP

#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>
#include "script/LuaScheduledScript.hpp"

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
#endif

#endif

//class defination of World so we can use a World pointer
class World;

/**
*this struct holds the information for the scripts which should be called
*/
struct ScriptData {
    uint32_t minCycleTime; /**< the minimal time before the next function call is invoked */
    uint32_t maxCycleTime; /**< the maximal time before the next function call is invoked */
    uint32_t nextCycleTime; /**< the exactly time when the next function call is invoked */
    uint32_t lastCycleTime; /**< the time of the last function call */
    std::string functionName; /**< the name of the function which should be called */
    std::string scriptName; /**< the correct filename of the script for the function */
    boost::shared_ptr<LuaScheduledScript>scriptptr; /**< the pointer to the script */

    /**
    *a small standard constructor
    */
    ScriptData() {
        minCycleTime = 0;
        maxCycleTime = 0;
        nextCycleTime = 0;
        lastCycleTime = 0;
        functionName = "";
        scriptName = "";
    }
    /**a standard constructor for initializing with values*/
    ScriptData(uint32_t minCT, uint32_t maxCT, uint32_t nextCT, uint32_t lastCT, std::string fname, std::string sname) {
        minCycleTime = minCT;
        maxCycleTime = maxCT;
        nextCycleTime = nextCT;
        lastCycleTime = lastCT;
        functionName = fname;
        scriptName = sname;
    }
};

/**
*this class loads a table with scripts
*and calls the scriptfunctions in different time intervalls
*/
class ScheduledScriptsTable {

public:

    /**
    *constructor for the table object
    *
    *loads the data from the table on construction
    *@param cw a pointer to the current gameworld
    */
    ScheduledScriptsTable();

    /**
    *the destructor, deletes the maps with data
    */
    ~ScheduledScriptsTable();

    /**
    *reloads all the tables
    */
    void reload();

    /**
    *inline function if all loading where sucessfully
    */
    inline bool dataOK() {
        return m_dataOk;
    }

    /**
    *invokes a new cycle
    *
    *@return true if a script was called otherwise false
    */
    bool nextCycle();

    /**
    *function which adds a new scheduled script on the right position
    *@param data the new script which should be added.
    *@return true if the adding was sucessfully otherwise false
    */
    bool addData(ScriptData data);

private:

    /**a list of all the scripts and there execution cycle. ordered by the next execution time*/
    std::list<ScriptData> m_table;

    /**the current cycle*/
    uint32_t currentCycle;

    /** a boolean which holds if loading was succesfully*/
    bool m_dataOk;

    void clearOldTable();
};
#endif
