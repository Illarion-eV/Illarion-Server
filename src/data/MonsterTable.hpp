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

#ifndef MONSTER_TABLE_HPP
#define MONSTER_TABLE_HPP

#include "script/LuaMonsterScript.hpp"

#include <TableStructs.hpp>
#include <boost/unordered_map.hpp>

/**
 * @brief Table for monster type definitions and scripts
 *
 * Loads monster data from the database "monster" table, providing access to
 * monster properties including:
 * - Names (German and English)
 * - Race and appearance properties (size range)
 * - Combat properties (HP, attack capability, self-healing)
 * - Movement type
 * - Associated Lua scripts for behavior
 *
 * This table uses the older direct database loading approach rather than
 * inheriting from StructTable. Monster scripts are loaded directly and
 * quest scripts are integrated during construction.
 *
 * Database table: monster
 */
class MonsterTable {
public:
    /**
     * @brief Constructor - loads monster data and scripts from database
     *
     * Queries the monster table and loads all monster definitions along with
     * their associated scripts. Also loads quest scripts from QuestNodeTable.
     */
    MonsterTable();

    /**
     * @brief Check if table data loaded successfully
     * @return true if data is valid, false if loading failed
     */
    [[nodiscard]] inline auto isDataOK() const -> bool { return dataOK; }

    /**
     * @brief Check if monster type exists
     * @param id Monster type ID
     * @return true if monster exists, false otherwise
     */
    [[nodiscard]] auto exists(TYPE_OF_CHARACTER_ID id) const -> bool;

    /**
     * @brief Access monster data by ID
     * @param id Monster type ID
     * @return Const reference to MonsterStruct
     * @note Creates default entry if ID not found
     */
    auto operator[](TYPE_OF_CHARACTER_ID id) -> const MonsterStruct &;

private:
    using TABLE = boost::unordered_map<TYPE_OF_CHARACTER_ID, MonsterStruct>;
    TABLE table;     ///< Map of monster ID to monster data
    bool dataOK = false;  ///< Whether data loaded successfully
};
#endif
