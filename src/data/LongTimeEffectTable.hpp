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

#ifndef LONG_TIME_EFFECT_TABLE_HPP
#define LONG_TIME_EFFECT_TABLE_HPP

#include "TableStructs.hpp"
#include "data/ScriptStructTable.hpp"
#include "script/LuaLongTimeEffectScript.hpp"

/**
 * @brief Table for long-term status effects and buffs/debuffs
 *
 * Loads long-time effect data from the database "longtimeeffects" table,
 * defining persistent effects that can be applied to characters:
 * - Buffs (stat boosts, damage shields, etc.)
 * - Debuffs (poisons, curses, stat penalties)
 * - Environmental effects (cold, heat, underwater)
 * - Timed conditions (paralysis, sleep, confusion)
 *
 * Each effect has an ID, name, and associated Lua script that implements
 * the effect's behavior (applying stat changes, dealing damage over time,
 * handling removal conditions, etc.).
 *
 * Effects are applied to characters with a duration and can stack or
 * be unique depending on script logic.
 *
 * Database table: longtimeeffects
 * Columns: lte_effectid, lte_effectname, lte_scriptname
 * Script type: LuaLongTimeEffectScript
 */
class LongTimeEffectTable : public ScriptStructTable<uint16_t, LongTimeEffectStruct, LuaLongTimeEffectScript> {
public:
    /**
     * @brief Get database table name
     * @return "longtimeeffects"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the longtimeeffects table
     * @return Vector containing effect table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract effect ID from database row
     * @param row Database result row
     * @return Effect ID from lte_effectid column
     */
    auto assignId(const Database::ResultTuple &row) -> uint16_t override;

    /**
     * @brief Parse database row into LongTimeEffectStruct
     * @param row Database result row
     * @return Populated LongTimeEffectStruct with effect ID and name
     */
    auto assignTable(const Database::ResultTuple &row) -> LongTimeEffectStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from lte_scriptname column
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif
