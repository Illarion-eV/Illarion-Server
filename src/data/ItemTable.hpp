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

#include "data/QuestScriptStructTable.hpp"
#include "script/LuaItemScript.hpp"
#include "TableStructs.hpp"

class ItemTable : public QuestScriptStructTable<TYPE_OF_ITEM_ID, ItemStruct, LuaItemScript> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID override;
    auto assignTable(const Database::ResultTuple &row) -> ItemStruct override;
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
    auto getQuestScripts() -> NodeRange override;

private:
    //TYPE_OF_ITEM_ID calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> &visited, std::map<TYPE_OF_ITEM_ID, bool> &assigned);
};

#endif

