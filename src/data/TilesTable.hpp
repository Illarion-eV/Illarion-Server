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

#ifndef TILES_TABLE_HPP
#define TILES_TABLE_HPP

#include "TableStructs.hpp"
#include "data/ScriptStructTable.hpp"
#include "script/LuaTileScript.hpp"
#include "types.hpp"

/**
 * @brief Table for tile type definitions and properties
 *
 * Loads tile data from the database "tiles" table, defining the properties of
 * different ground tile types in the game world:
 * - Passability (whether characters can walk on the tile)
 * - Walking cost (movement penalty/bonus)
 * - Localized names (German and English)
 * - Optional tile scripts for special behavior
 *
 * Tile types define the base terrain (grass, stone, water, etc.) and their
 * traversal properties. Scripts can add special effects when characters
 * walk on specific tile types.
 *
 * Database table: tiles
 * Columns: til_id, til_isnotpassable, til_german, til_english, til_walkingcost, til_script
 * Script type: LuaTileScript
 */
class TilesTable : public ScriptStructTable<TYPE_OF_TILE_ID, TilesStruct, LuaTileScript> {
public:
    /**
     * @brief Get database table name
     * @return "tiles"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the tiles table
     * @return Vector containing tile table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract tile ID from database row
     * @param row Database result row
     * @return Tile ID from til_id column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_TILE_ID override;

    /**
     * @brief Parse database row into TilesStruct
     * @param row Database result row
     * @return Populated TilesStruct with passability, names, and walking cost
     */
    auto assignTable(const Database::ResultTuple &row) -> TilesStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from til_script column, or empty string
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif
