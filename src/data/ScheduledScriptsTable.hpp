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

/**
 * @brief Data for a scheduled script execution
 *
 * Contains all timing and identification information for a script that
 * runs periodically on the server.
 */
struct ScriptData {
    uint32_t minCycleTime = 0;      ///< Minimum cycles between executions
    uint32_t maxCycleTime = 0;      ///< Maximum cycles between executions
    uint32_t nextCycleTime = 0;     ///< Next cycle when script should run
    uint32_t lastCycleTime = 0;     ///< Last cycle when script ran
    std::string functionName;       ///< Lua function to call
    std::string scriptName;         ///< Script filename
    std::shared_ptr<LuaScheduledScript> scriptptr;  ///< Loaded script instance

    ScriptData() = default;

    /**
     * @brief Construct script data with all parameters
     * @param minCT Minimum cycle time
     * @param maxCT Maximum cycle time
     * @param nextCT Next execution cycle
     * @param lastCT Last execution cycle
     * @param fname Function name to call
     * @param sname Script filename
     */
    ScriptData(uint32_t minCT, uint32_t maxCT, uint32_t nextCT, uint32_t lastCT, std::string fname, std::string sname)
            : minCycleTime(minCT), maxCycleTime(maxCT), nextCycleTime(nextCT), lastCycleTime(lastCT),
              functionName(std::move(fname)), scriptName(std::move(sname)) {}
};

/**
 * @brief Table for periodically executed scripts
 *
 * Loads and manages scripts that run on a schedule from the database
 * "scheduledscripts" table. Scheduled scripts enable:
 * - World maintenance tasks (cleanup, spawning)
 * - Periodic events (day/night cycle, weather)
 * - Timed game mechanics (regeneration, decay)
 * - Background processes (economy updates, NPC behavior)
 *
 * Scripts are stored in a priority queue sorted by next execution time.
 * Each cycle (game tick), due scripts are executed and rescheduled with
 * a random interval between min and max cycle time.
 *
 * @note Uses older direct database loading rather than StructTable
 * @note Limited to 200 script executions per cycle to prevent runaway execution
 *
 * Database table: scheduledscripts
 */
class ScheduledScriptsTable {
public:
    /**
     * @brief Constructor - loads scheduled scripts from database
     *
     * Queries the scheduledscripts table and loads all scripts with their
     * timing parameters.
     */
    ScheduledScriptsTable();

    /**
     * @brief Check if table data loaded successfully
     * @return true if data is valid, false if loading failed
     */
    [[nodiscard]] inline auto isDataOK() const -> bool { return m_dataOk; }

    /**
     * @brief Advance to next cycle and execute due scripts
     *
     * Increments the cycle counter, executes all scripts whose nextCycleTime
     * has been reached, and reschedules them with a new random interval.
     * Limited to 200 executions per cycle for safety.
     */
    void nextCycle();

    /**
     * @brief Add or reschedule a script in the execution queue
     *
     * Inserts script into the queue at the appropriate position based on
     * its nextCycleTime, maintaining sorted order.
     *
     * @param data Script data to add
     */
    void addData(const ScriptData &data);

private:
    /**
     * @brief Load scheduled scripts from database
     *
     * Called by constructor to populate the script list.
     */
    void reload();

    std::list<ScriptData> m_table;  ///< Sorted list of scheduled scripts
    uint32_t currentCycle{0};       ///< Current server cycle/tick count
    bool m_dataOk{false};           ///< Whether data loaded successfully

    /**
     * @brief Clear old table data before reload
     */
    void clearOldTable();
};
#endif
