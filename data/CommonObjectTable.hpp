/*
 * Illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of Illarionserver.
 *
 * Illarionserver  is  free  software:  you can redistribute it and/or modify it
 * under the terms of the  GNU  General  Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Illarionserver is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY;  without  even  the  implied  warranty  of  MERCHANTABILITY  or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _COMMON_OBJECT_TABLE_HPP_
#define _COMMON_OBJECT_TABLE_HPP_

#include "data/QuestScriptStructTable.hpp"
#include "script/LuaItemScript.hpp"
#include "TableStructs.hpp"

class CommonObjectTable : public QuestScriptStructTable<TYPE_OF_ITEM_ID, CommonStruct, LuaItemScript> {
public:
    virtual std::string getTableName();
    virtual std::vector<std::string> getColumnNames();
    virtual TYPE_OF_ITEM_ID assignId(const Database::ResultTuple &row);
    virtual CommonStruct assignTable(const Database::ResultTuple &row);
    virtual std::string assignScriptName(const Database::ResultTuple &row);
    virtual NodeRange getQuestScripts();

private:
    //TYPE_OF_ITEM_ID calcInfiniteRot(TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> &visited, std::map<TYPE_OF_ITEM_ID, bool> &assigned);
};

#endif

