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

#include "data/ScriptStructTable.hpp"
#include "types.hpp"
#include "TableStructs.hpp"
#include "script/LuaWeaponScript.hpp"

class WeaponObjectTable : public ScriptStructTable<TYPE_OF_ITEM_ID, WeaponStruct, LuaWeaponScript> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;
    auto assignTable(const Database::ResultTuple &row) -> WeaponStruct override;
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif

