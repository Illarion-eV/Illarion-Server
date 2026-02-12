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
#ifndef QUEST_TABLE_HPP
#define QUEST_TABLE_HPP

#include "data/ScriptStructTable.hpp"
#include "globals.hpp"
#include "script/LuaQuestScript.hpp"
#include "types.hpp"

#include <map>

/**
 * @brief Empty struct for quest data (all data is in scripts)
 *
 * Quest logic is entirely implemented in Lua scripts, so no
 * additional data structure is needed in C++.
 */
struct QuestStruct {};

/**
 * @brief Table for quest definitions and quest scripts
 *
 * Loads quest data from the database "quests" table, managing the questing
 * system including:
 * - Quest definitions and IDs
 * - Quest scripts implementing quest logic
 * - Quest start locations for proximity detection
 * - Spatial indexing for finding nearby quests
 *
 * Each quest has a start position (defined in its Lua script) that determines
 * where it becomes available to players. The table maintains a spatial index
 * for efficient lookup of quests within range of a player's position.
 *
 * @note ScriptParameter is TYPE_OF_QUEST_ID (not QuestStruct), so scripts
 *       are constructed with quest ID only
 *
 * Database table: quests
 * Columns: qst_id, qst_script
 * Script type: LuaQuestScript
 */
class QuestTable : public ScriptStructTable<TYPE_OF_QUEST_ID, QuestStruct, LuaQuestScript, TYPE_OF_QUEST_ID> {
private:
    using Base = ScriptStructTable<TYPE_OF_QUEST_ID, QuestStruct, LuaQuestScript, TYPE_OF_QUEST_ID>;
    using quest_starts_type = std::multimap<position, TYPE_OF_QUEST_ID, PositionComparison>;

public:
    /**
     * @brief Get database table name
     * @return "quests"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the quests table
     * @return Vector containing quest table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract quest ID from database row
     * @param row Database result row
     * @return Quest ID from qst_id column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_QUEST_ID override;

    /**
     * @brief Parse database row into QuestStruct
     * @param row Database result row
     * @return Empty QuestStruct (no data stored)
     */
    auto assignTable(const Database::ResultTuple &row) -> QuestStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from qst_script column
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;

    /**
     * @brief Reload scripts and build spatial index of quest starts
     *
     * After loading quest scripts, queries each script for its start position
     * and builds a spatial index for efficient proximity queries.
     */
    void reloadScripts() override;

    using QuestStartMap = std::map<TYPE_OF_QUEST_ID, position>;

    /**
     * @brief Find quests within range of a position
     *
     * Uses the spatial index to efficiently find all quests whose start
     * positions are within the specified radius of the given position.
     * Uses Manhattan distance (|dx| + |dy|) for the 2D check.
     *
     * @param pos Center position to search from
     * @param radius Maximum distance (in tiles) from center
     * @return Map of quest IDs to their start positions within range
     */
    auto getQuestsInRange(const position &pos, Coordinate radius) const -> QuestStartMap;

private:
    quest_starts_type questStarts;  ///< Spatial index: position -> quest IDs
};

#endif
