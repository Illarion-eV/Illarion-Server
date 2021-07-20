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

#ifndef SCHEDULED_SCRIPTS_TABLE_HPP
#define SCHEDULED_SCRIPTS_TABLE_HPP

#include "script/LuaScheduledScript.hpp"

#include <boost/unordered_map.hpp>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

class World;

struct ScriptData {
    uint32_t minCycleTime = 0;
    uint32_t maxCycleTime = 0;
    uint32_t nextCycleTime = 0;
    uint32_t lastCycleTime = 0;
    std::string functionName;
    std::string scriptName;
    std::shared_ptr<LuaScheduledScript> scriptptr;

    ScriptData() = default;
    ScriptData(uint32_t minCT, uint32_t maxCT, uint32_t nextCT, uint32_t lastCT, std::string fname, std::string sname)
            : minCycleTime(minCT), maxCycleTime(maxCT), nextCycleTime(nextCT), lastCycleTime(lastCT),
              functionName(std::move(fname)), scriptName(std::move(sname)) {}
};

class ScheduledScriptsTable {
public:
    ScheduledScriptsTable();

    [[nodiscard]] inline auto isDataOK() const -> bool { return m_dataOk; }

    void nextCycle();

    void addData(ScriptData data);

private:
    void reload();

    std::list<ScriptData> m_table;
    uint32_t currentCycle{0};
    bool m_dataOk{false};

    void clearOldTable();
};
#endif
