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

#include "data/ScheduledScriptsTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "Logger.hpp"
#include "Random.hpp"

ScheduledScriptsTable::ScheduledScriptsTable() : currentCycle(0), m_dataOk(false) {
    reload();
}

ScheduledScriptsTable::~ScheduledScriptsTable() {
    /**@todo destructor implementatiotn*/
    m_table.clear();
}

bool ScheduledScriptsTable::nextCycle() {
    currentCycle++;
    ScriptData data; /**< holds the current task*/
    int emexit = 0; /**< emergency counter which breaks the loop if to much execution*/

    while (!m_table.empty() && (emexit < 200) && (m_table.front().nextCycleTime <= currentCycle)) {
        emexit++;

        if (emexit>=200) {
            break;    /**< emergency exit so we can't create an endless loop */
        }

        if (!m_table.empty()) {
            data = m_table.front(); /**< copy the first data in data*/
            m_table.pop_front(); /**< deletes the first task */

            if (data.scriptptr) {
                /**calculate the next time where the script is invoked */
                data.nextCycleTime = currentCycle + Random::uniform(data.minCycleTime, data.maxCycleTime);

                /**call the script function */
                data.scriptptr->callFunction(data.functionName,currentCycle,data.lastCycleTime,data.nextCycleTime);

                data.lastCycleTime = currentCycle; /**< script was runned and so we can change the lastCycleTime*/

                addData(data); /**< insert the script in the list again*/
                return true;
            }
        }
    }

    return false;
}

bool ScheduledScriptsTable::addData(ScriptData data) {
    Logger::writeMessage("schedscripts","insert new Task task.nextCycle: " + Logger::toString(data.nextCycleTime)  + " current Cycle: " + Logger::toString(currentCycle));
    std::list<ScriptData>::iterator it;
    bool inserted = false;

    if (data.nextCycleTime <= currentCycle) {
        data.nextCycleTime = currentCycle + Random::uniform(data.minCycleTime, data.maxCycleTime);
    }

    for (it = m_table.begin(); it != m_table.end(); ++it) {
        if ((*it).nextCycleTime > data.nextCycleTime) {
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
#ifdef DataConnect_DEBUG
    std::cout << "ScheduledScriptsTable: reload" << std::endl;
#endif

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

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                tmpRecord.minCycleTime = (uint32_t)((*itr)["sc_mincycletime"].as<uint32_t>());
                tmpRecord.maxCycleTime = (uint32_t)((*itr)["sc_maxcycletime"].as<uint32_t>());
                tmpRecord.nextCycleTime = 0;

                if (!((*itr)["sc_scriptname"].is_null()) && !((*itr)["sc_functionname"].is_null())) {
                    tmpRecord.functionName = ((*itr)["sc_functionname"].as<std::string>());
                    tmpRecord.scriptName = ((*itr)["sc_scriptname"].as<std::string>());

                    try {
                        std::shared_ptr<LuaScheduledScript> tmpScript(new LuaScheduledScript(tmpRecord.scriptName));
                        tmpRecord.scriptptr = tmpScript;
                        addData(tmpRecord);
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading scheduled script: " + tmpRecord.scriptName + ":\n" + e.what() + "\n");
                    }
                }
            }
        }

        m_dataOk = true;
    } catch (std::exception &e) {
        std::cerr << "exception: " << e.what() << std::endl;
        m_dataOk = false;
    }
}

void ScheduledScriptsTable::clearOldTable() {
    m_table.clear();
}

