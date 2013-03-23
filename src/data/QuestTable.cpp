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

#include "data/QuestTable.hpp"

std::string QuestTable::getTableName() {
    return "quests";
}

std::vector<std::string> QuestTable::getColumnNames() {
    return {
        "qst_id",
        "qst_script"
    };
}

TYPE_OF_QUEST_ID QuestTable::assignId(const Database::ResultTuple &row) {
    return row["qst_id"].as<TYPE_OF_QUEST_ID>();
}

QuestStruct QuestTable::assignTable(const Database::ResultTuple &row) {
    return QuestStruct();
}

std::string QuestTable::assignScriptName(const Database::ResultTuple &row) {
    return row["qst_script"].as<std::string>("");
}

