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

#include "data/QuestTable.hpp"

#include "constants.hpp"

auto QuestTable::getTableName() -> std::string {
    return "quests";
}

auto QuestTable::getColumnNames() -> std::vector<std::string> {
    return {"qst_id", "qst_script"};
}

auto QuestTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_QUEST_ID {
    return row["qst_id"].as<TYPE_OF_QUEST_ID>();
}

auto QuestTable::assignTable(const Database::ResultTuple &row) -> QuestStruct {
    return QuestStruct();
}

auto QuestTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["qst_script"].as<std::string>("");
}

void QuestTable::reloadScripts() {
    Base::reloadScripts();

    questStarts.clear();

    for (const auto &quest : *this) {
        const TYPE_OF_QUEST_ID id = quest.first;
        auto questScript = script(id);

        if (questScript != nullptr) {
            try {
                questStarts.insert(std::make_pair(questScript->start(), id));
            } catch (std::logic_error &e) {
            }
        }
    }
}

auto QuestTable::getQuestsInRange(const position &pos, int radius) const -> QuestStartMap {
    const auto begin = questStarts.upper_bound(position(pos.x - radius - 1, 0, 0));
    const auto end = questStarts.upper_bound(position(pos.x + radius + 1, 0, 0));

    QuestStartMap result;

    for (auto it = begin; it != end; ++it) {
        const auto id = it->second;
        const auto &start = it->first;

        short int dx = start.x - pos.x;
        short int dy = start.y - pos.y;
        short int dz = start.z - pos.z;

        if ((abs(dx) + abs(dy) <= radius) && (-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            result[id] = start;
        }
    }

    return result;
}
