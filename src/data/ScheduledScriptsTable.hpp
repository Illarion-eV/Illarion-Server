/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Affero General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SCHEDULED_SCRIPTS_TABLE_HPP_
#define _SCHEDULED_SCRIPTS_TABLE_HPP_

#include <string>
#include <map>
#include <list>
#include <memory>
#include <boost/unordered_map.hpp>
#include "script/LuaScheduledScript.hpp"

class World;

struct ScriptData {
    uint32_t minCycleTime;
    uint32_t maxCycleTime;
    uint32_t nextCycleTime;
    uint32_t lastCycleTime;
    std::string functionName;
    std::string scriptName;
    std::shared_ptr<LuaScheduledScript>scriptptr;

    ScriptData(): functionName(""), scriptName("") {
        minCycleTime = 0;
        maxCycleTime = 0;
        nextCycleTime = 0;
        lastCycleTime = 0;
    }
    ScriptData(uint32_t minCT, uint32_t maxCT, uint32_t nextCT, uint32_t lastCT, std::string fname, std::string sname): functionName(fname), scriptName(sname) {
        minCycleTime = minCT;
        maxCycleTime = maxCT;
        nextCycleTime = nextCT;
        lastCycleTime = lastCT;
    }
};

class ScheduledScriptsTable {
public:
    ScheduledScriptsTable();
    ~ScheduledScriptsTable();

    inline bool isDataOK() const {
        return m_dataOk;
    }

    bool nextCycle();

    bool addData(ScriptData data);

private:
    void reload();

    std::list<ScriptData> m_table;
    uint32_t currentCycle;
    bool m_dataOk;

    void clearOldTable();
};
#endif

