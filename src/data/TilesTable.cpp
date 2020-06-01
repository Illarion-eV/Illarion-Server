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

#include "data/TilesTable.hpp"

#include "constants.hpp"

auto TilesTable::getTableName() -> std::string { return "tiles"; }

auto TilesTable::getColumnNames() -> std::vector<std::string> {
    return {"til_id", "til_isnotpassable", "til_german", "til_english", "til_walkingcost", "til_script"};
}

auto TilesTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_TILE_ID {
    return row["til_id"].as<TYPE_OF_TILE_ID>();
}

auto TilesTable::assignTable(const Database::ResultTuple &row) -> TilesStruct {
    TilesStruct tile;
    tile.flags = row["til_isnotpassable"].as<bool>() ? FLAG_BLOCKPATH : 0;
    tile.German = row["til_german"].as<std::string>();
    tile.English = row["til_english"].as<std::string>();
    tile.walkingCost = row["til_walkingcost"].as<TYPE_OF_WALKINGCOST>();
    return tile;
}

auto TilesTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["til_script"].as<std::string>("");
}
