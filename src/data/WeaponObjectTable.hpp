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

#ifndef WEAPON_OBJECT_TABLE_HPP
#define WEAPON_OBJECT_TABLE_HPP

#include "TableStructs.hpp"
#include "data/ScriptStructTable.hpp"
#include "script/LuaWeaponScript.hpp"
#include "types.hpp"

/**
 * @brief Table for weapon combat properties and scripts
 *
 * Loads weapon data from the database "weapon" table, providing access to
 * combat statistics for items that function as weapons:
 * - Combat stats (attack, defence, accuracy, range)
 * - Weapon classification (type, ammunition type)
 * - Combat mechanics (action points, magic disturbance)
 * - Special effects (poison strength)
 * - Optional fighting scripts for custom weapon behavior
 *
 * Weapon properties are looked up by item ID, allowing items to function
 * as weapons when they have an entry in this table.
 *
 * Database table: weapon
 * Script type: LuaWeaponScript
 */
class WeaponObjectTable : public ScriptStructTable<TYPE_OF_ITEM_ID, WeaponStruct, LuaWeaponScript> {
public:
    /**
     * @brief Get database table name
     * @return "weapon"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the weapon table
     * @return Vector containing weapon table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract item ID from database row
     * @param row Database result row
     * @return Item ID from wp_itemid column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;

    /**
     * @brief Parse database row into WeaponStruct
     * @param row Database result row
     * @return Populated WeaponStruct with combat properties
     */
    auto assignTable(const Database::ResultTuple &row) -> WeaponStruct override;

    /**
     * @brief Extract fighting script filename from database row
     * @param row Database result row
     * @return Script filename from wp_fightingscript column, or empty string
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif
