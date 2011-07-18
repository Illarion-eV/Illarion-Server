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

#include "data/MonsterAttackTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "types.hpp"

MonsterAttackTable::MonsterAttackTable() : m_dataOk(false) {
    reload();
}

MonsterAttackTable::~MonsterAttackTable() {
    clearOldTable();
}

void MonsterAttackTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"Trying to reload MonsterAttackTable!"<<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("monsterattack", "mat_race_type");
        query.addColumn("monsterattack", "mat_attack_type");
        query.addColumn("monsterattack", "mat_attack_value");
        query.addColumn("monsterattack", "mat_actionpointslost");
        query.addServerTable("monsterattack");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            AttackBoni data;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                data.attackType = (uint8_t)((*itr)["mat_attack_type"].as<int16_t>());
                data.attackValue = (int16_t)((*itr)["mat_attack_value"].as<int16_t>());
                data.actionPointsLost = (int16_t)((*itr)["mat_actionpointslost"].as<int16_t>());
                raceAttackBoni[(int16_t)((*itr)["mat_race_type"].as<int16_t>())] = data;
            }
        }

        m_dataOk = true;

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into MonsterAttackTable" << std::endl;
#endif
    } catch (...) {
        m_dataOk = false;
    }
}

bool MonsterAttackTable::find(Character::race_type race, AttackBoni &ret) {
    TABLE::iterator iterat;
    iterat = raceAttackBoni.find((int16_t)race);

    if (iterat == raceAttackBoni.end()) {
        return false;
    } else {
        ret = (*iterat).second;
        return true;
    }

    return false;
}

void MonsterAttackTable::clearOldTable() {
    raceAttackBoni.clear();
}
