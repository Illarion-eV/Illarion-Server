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

std::string TilesTable::getTableName() {
    return "tiles";
}

std::vector<std::string> TilesTable::getColumnNames() {
    return {
        "til_id",
        "til_isnotpassable",
        "til_isnottransparent",
        "til_isnotpenetrateable",
        "til_specialtile",
        "til_groundlevel",
        "til_german",
        "til_english",
        "til_walkingcost",
        "til_script"
    };
}

TYPE_OF_TILE_ID TilesTable::assignId(const Database::ResultTuple &row) {
    return row["til_id"].as<TYPE_OF_TILE_ID>();
}

TilesStruct TilesTable::assignTable(const Database::ResultTuple &row) {
    TilesStruct tile;
    tile.flags = uint8_t(row["til_groundlevel"].as<uint16_t>());
    tile.flags |= row["til_isnotpassable"].as<bool>() ? FLAG_PASSABLE : 0;
    tile.flags |= row["til_isnottransparent"].as<bool>() ? FLAG_TRANSPARENT : 0;
    tile.flags |= row["til_isnotpenetrateable"].as<bool>() ? FLAG_TRANSPARENT : 0;
    tile.flags |= row["til_specialtile"].as<bool>() ? FLAG_SPECIALITEM : 0;
    tile.German = row["til_german"].as<std::string>();
    tile.English = row["til_english"].as<std::string>();
    tile.walkingCost = TYPE_OF_WALKINGCOST(row["til_walkingcost"].as<int16_t>());
    return tile;
}

std::string TilesTable::assignScriptName(const Database::ResultTuple &row) {
    return row["til_script"].as<std::string>("");
}

