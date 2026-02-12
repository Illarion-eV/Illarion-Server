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

#ifndef COMMON_OBJECT_TABLE_HPP
#define COMMON_OBJECT_TABLE_HPP

#include "TableStructs.hpp"
#include "data/QuestScriptStructTable.hpp"
#include "script/LuaItemScript.hpp"

/**
 * @brief Table for item definitions and properties
 *
 * Loads item data from the database "items" table, providing access to all
 * item properties including:
 * - Physical properties (volume, weight)
 * - Aging/decay properties (aging speed, rot target, inventory decay)
 * - Display properties (brightness)
 * - Economic properties (worth, stack limits)
 * - Localized names and descriptions
 * - Rarity and level requirements
 *
 * Supports both main item scripts and quest-specific item scripts via the
 * QuestScriptStructTable base class.
 *
 * Database table: items
 * Script type: LuaItemScript
 */
class ItemTable : public QuestScriptStructTable<TYPE_OF_ITEM_ID, ItemStruct, LuaItemScript> {
public:
    /**
     * @brief Get database table name
     * @return "items"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the items table
     * @return Vector containing all item table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract item ID from database row
     * @param row Database result row
     * @return Item ID from itm_id column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;

    /**
     * @brief Parse database row into ItemStruct
     * @param row Database result row
     * @return Populated ItemStruct with all item properties
     */
    auto assignTable(const Database::ResultTuple &row) -> ItemStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from itm_script column
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;

    /**
     * @brief Get quest script nodes for items
     * @return Iterator range over item quest nodes
     */
    auto getQuestScripts() -> NodeRange override;

private:
    // TYPE_OF_ITEM_ID calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> &visited,
    // std::map<TYPE_OF_ITEM_ID, bool> &assigned);
};

#endif
