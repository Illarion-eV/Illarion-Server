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

#include "data/MonsterAttackTable.hpp"

auto MonsterAttackTable::getTableName() -> std::string { return "monsterattack"; }

auto MonsterAttackTable::getColumnNames() -> std::vector<std::string> {
    return {"mat_race_type", "mat_attack_type", "mat_attack_value", "mat_actionpointslost"};
}

auto MonsterAttackTable::assignId(const Database::ResultTuple &row) -> uint16_t {
    return uint16_t(row["mat_race_type"].as<int16_t>());
}

auto MonsterAttackTable::assignTable(const Database::ResultTuple &row) -> AttackBoni {
    AttackBoni attack;
    attack.attackType = uint8_t(row["mat_attack_type"].as<int16_t>());
    attack.attackValue = row["mat_attack_value"].as<int16_t>();
    attack.actionPointsLost = row["mat_actionpointslost"].as<int16_t>();
    return attack;
}
