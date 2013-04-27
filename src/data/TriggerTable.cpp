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

#include "data/TriggerTable.hpp"

std::string TriggerTable::getTableName() {
    return "triggerfields";
}

std::vector<std::string> TriggerTable::getColumnNames() {
    return {
        "tgf_posx",
        "tgf_posy",
        "tgf_posz",
        "tgf_script"
    };
}

position TriggerTable::assignId(const Database::ResultTuple &row) {
    return position(
               row["tgf_posx"].as<int16_t>(),
               row["tgf_posy"].as<int16_t>(),
               row["tgf_posz"].as<int16_t>()
           );
}

TriggerStruct TriggerTable::assignTable(const Database::ResultTuple &row) {
    TriggerStruct trigger;
    trigger.pos = assignId(row);
    return trigger;
}

std::string TriggerTable::assignScriptName(const Database::ResultTuple &row) {
    return row["tgf_script"].as<std::string>("");
}

auto TriggerTable::getQuestScripts() -> NodeRange {
    return QuestNodeTable::getInstance().getTriggerNodes();
}

