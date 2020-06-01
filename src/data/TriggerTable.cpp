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

#include "data/TriggerTable.hpp"

auto TriggerTable::getTableName() -> std::string {
    return "triggerfields";
}

auto TriggerTable::getColumnNames() -> std::vector<std::string> {
    return {"tgf_posx", "tgf_posy", "tgf_posz", "tgf_script"};
}

auto TriggerTable::assignId(const Database::ResultTuple &row) -> position {
    return position(row["tgf_posx"].as<int16_t>(), row["tgf_posy"].as<int16_t>(), row["tgf_posz"].as<int16_t>());
}

auto TriggerTable::assignTable(const Database::ResultTuple &row) -> TriggerStruct {
    TriggerStruct trigger;
    trigger.pos = assignId(row);
    return trigger;
}

auto TriggerTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["tgf_script"].as<std::string>("");
}

auto TriggerTable::getQuestScripts() -> NodeRange {
    return QuestNodeTable::getInstance().getTriggerNodes();
}
