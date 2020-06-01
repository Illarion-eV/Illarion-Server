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

class TilesTable : public ScriptStructTable<TYPE_OF_TILE_ID, TilesStruct, LuaTileScript> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_TILE_ID override;
    auto assignTable(const Database::ResultTuple &row) -> TilesStruct override;
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif
