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
#ifndef QUEST_TABLE_HPP
#define QUEST_TABLE_HPP

#include "data/ScriptStructTable.hpp"
#include "globals.hpp"
#include "script/LuaQuestScript.hpp"
#include "types.hpp"

#include <map>

struct QuestStruct {};

class QuestTable : public ScriptStructTable<TYPE_OF_QUEST_ID, QuestStruct, LuaQuestScript, TYPE_OF_QUEST_ID> {
private:
    using Base = ScriptStructTable<TYPE_OF_QUEST_ID, QuestStruct, LuaQuestScript, TYPE_OF_QUEST_ID>;
    using quest_starts_type = std::multimap<position, TYPE_OF_QUEST_ID, PositionComparison>;

public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_QUEST_ID override;
    auto assignTable(const Database::ResultTuple &row) -> QuestStruct override;
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
    void reloadScripts() override;

    using QuestStartMap = std::map<TYPE_OF_QUEST_ID, position>;
    auto getQuestsInRange(const position &pos, Coordinate radius) const -> QuestStartMap;

private:
    quest_starts_type questStarts;
};

#endif
