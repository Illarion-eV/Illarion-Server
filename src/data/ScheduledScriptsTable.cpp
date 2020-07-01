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

#include "data/ScheduledScriptsTable.hpp"

#include "Logger.hpp"
#include "Random.hpp"
#include "db/Result.hpp"
#include "db/SelectQuery.hpp"

#include <iostream>

ScheduledScriptsTable::ScheduledScriptsTable() { reload(); }

auto ScheduledScriptsTable::nextCycle() -> bool {
    currentCycle++;
    ScriptData data; /**< holds the current task*/
    constexpr auto scriptLimit = 200;
    int emexit = 0; /**< emergency counter which breaks the loop if too much execution*/

    while (!m_table.empty() && (emexit++ < scriptLimit) && (m_table.front().nextCycleTime <= currentCycle)) {
        if (!m_table.empty()) {
            data = m_table.front(); /**< copy the first data in data*/
            m_table.pop_front();    /**< deletes the first task */

            if (data.scriptptr) {
                /**calculate the next time where the script is invoked */
                data.nextCycleTime = currentCycle + Random::uniform(data.minCycleTime, data.maxCycleTime);

                /**call the script function */
                data.scriptptr->callFunction(data.functionName, currentCycle, data.lastCycleTime, data.nextCycleTime);

                data.lastCycleTime = currentCycle; /**< script was runned and so we can change the lastCycleTime*/

                addData(data); /**< insert the script in the list again*/
                return true;
            }
        }
    }

    return false;
}

auto ScheduledScriptsTable::addData(ScriptData data) -> bool {
    Logger::debug(LogFacility::Script) << "insert new Task task.nextCycle: " << data.nextCycleTime
                                       << " current Cycle: " << currentCycle << Log::end;
    bool inserted = false;

    if (data.nextCycleTime <= currentCycle) {
        data.nextCycleTime = currentCycle + Random::uniform(data.minCycleTime, data.maxCycleTime);
    }

    for (auto it = m_table.begin(); it != m_table.end(); ++it) {
        if (it->nextCycleTime > data.nextCycleTime) {
            m_table.insert(it, data);
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        m_table.push_back(data);
    }

    return true;
}

void ScheduledScriptsTable::reload() {
    try {
        Database::SelectQuery query;
        query.addColumn("scheduledscripts", "sc_scriptname");
        query.addColumn("scheduledscripts", "sc_mincycletime");
        query.addColumn("scheduledscripts", "sc_maxcycletime");
        query.addColumn("scheduledscripts", "sc_functionname");
        query.addServerTable("scheduledscripts");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            ScriptData tmpRecord;

            for (const auto &row : results) {
                tmpRecord.minCycleTime = row["sc_mincycletime"].as<uint32_t>();
                tmpRecord.maxCycleTime = row["sc_maxcycletime"].as<uint32_t>();
                tmpRecord.nextCycleTime = 0;

                if (!row["sc_scriptname"].is_null() && !row["sc_functionname"].is_null()) {
                    tmpRecord.functionName = row["sc_functionname"].as<std::string>();
                    tmpRecord.scriptName = row["sc_scriptname"].as<std::string>();

                    try {
                        std::shared_ptr<LuaScheduledScript> tmpScript =
                                std::make_shared<LuaScheduledScript>(tmpRecord.scriptName);
                        tmpRecord.scriptptr = tmpScript;
                        addData(tmpRecord);
                    } catch (const ScriptException &e) {
                        Logger::error(LogFacility::Script)
                                << "Error while loading scheduled script: " << tmpRecord.scriptName << ": " << e.what()
                                << Log::end;
                    }
                }
            }
        }

        m_dataOk = true;
    } catch (std::exception &e) {
        Logger::error(LogFacility::Other) << "Exception in ScheduledScriptsTable::reload: " << e.what() << Log::end;
        m_dataOk = false;
    }
}

void ScheduledScriptsTable::clearOldTable() { m_table.clear(); }
