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

#ifndef ARMOR_OBJECT_TABLE_HPP
#define ARMOR_OBJECT_TABLE_HPP

#include "TableStructs.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"

/**
 * @brief Table for armor/protective equipment properties
 *
 * Loads armor data from the database "armor" table, providing access to
 * defensive properties for items that function as armor:
 * - Body parts covered (head, torso, arms, legs, etc.)
 * - Damage type resistances (puncture, stroke, thrust)
 * - Magic disturbance penalty
 * - Damage absorption and stiffness (movement penalty)
 * - Armor type classification
 *
 * Armor properties are looked up by item ID, allowing items to provide
 * protection when they have an entry in this table.
 *
 * Database table: armor
 */
class ArmorObjectTable : public StructTable<TYPE_OF_ITEM_ID, ArmorStruct> {
public:
    /**
     * @brief Get database table name
     * @return "armor"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the armor table
     * @return Vector containing armor table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract item ID from database row
     * @param row Database result row
     * @return Item ID from arm_itemid column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;

    /**
     * @brief Parse database row into ArmorStruct
     * @param row Database result row
     * @return Populated ArmorStruct with armor properties
     */
    auto assignTable(const Database::ResultTuple &row) -> ArmorStruct override;
};

#endif
