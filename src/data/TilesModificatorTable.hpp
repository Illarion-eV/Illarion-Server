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

#ifndef TILES_MODIFICATOR_TABLE_HPP
#define TILES_MODIFICATOR_TABLE_HPP

#include "TableStructs.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"

/**
 * @brief Table for item-based tile modifications
 *
 * Loads tile modificator data from the database "tilesmodificators" table,
 * defining how items placed on tiles modify their properties:
 * - Blocking movement (walls, fences, large objects)
 * - Special item flags (for quest/script purposes)
 * - Making impassable tiles passable (bridges over water)
 *
 * When items are placed in the world, their tile modificator properties
 * affect the tile's behavior. For example, a wall item makes the tile
 * impassable, while a bridge item makes water tiles passable.
 *
 * @note TODO: The handling of tile modificators is extremely unsafe.
 *       Errors in the database will corrupt data easily and uncontrollably.
 *
 * Database table: tilesmodificators
 * Columns: tim_itemid, tim_isnotpassable, tim_specialitem, tim_makepassable
 */
class TilesModificatorTable : public StructTable<TYPE_OF_ITEM_ID, TilesModificatorStruct> {
public:
    /**
     * @brief Get database table name
     * @return "tilesmodificators"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the tilesmodificators table
     * @return Vector containing tile modificator column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract item ID from database row
     * @param row Database result row
     * @return Item ID from tim_itemid column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;

    /**
     * @brief Parse database row into TilesModificatorStruct
     * @param row Database result row
     * @return Populated TilesModificatorStruct with modificator flags
     */
    auto assignTable(const Database::ResultTuple &row) -> TilesModificatorStruct override;

    /**
     * @brief Check if an item allows passage
     *
     * Determines if a tile with this item placed on it can be walked through.
     * An item is passable if it doesn't block the path OR if it explicitly
     * makes the tile passable (like a bridge).
     *
     * @param id Item ID to check
     * @return true if passable, false if blocks movement
     * @note Returns true if item ID not found in table
     */
    auto passable(TYPE_OF_ITEM_ID id) -> bool;
};

#endif
