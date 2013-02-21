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
#ifndef _QUEST_TABLE_HPP_
#define _QUEST_TABLE_HPP_

#include "data/ScriptStructTable.hpp"
#include "types.hpp"
#include "script/LuaQuestScript.hpp"

struct QuestStruct {
};

class QuestTable : public ScriptStructTable<TYPE_OF_QUEST_ID, QuestStruct, LuaQuestScript, TYPE_OF_QUEST_ID> {
public:
    virtual std::string getTableName();
    virtual std::vector<std::string> getColumnNames();
    virtual TYPE_OF_QUEST_ID assignId(const Database::ResultTuple &row);
    virtual QuestStruct assignTable(const Database::ResultTuple &row);
    virtual std::string assignScriptName(const Database::ResultTuple &row);
};

#endif

