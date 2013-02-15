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

#ifndef _MONSTER_ATTACK_TABLE_
#define _MONSTER_ATTACK_TABLE_

#include <string>
#include <boost/unordered_map.hpp>
#include "Character.hpp"

struct AttackBoni {
    uint8_t attackType;
    int16_t attackValue;
    int16_t actionPointsLost;
    AttackBoni() : attackType(0), attackValue(0), actionPointsLost(0) {}
};

class MonsterAttackTable {
public:
    MonsterAttackTable();
    ~MonsterAttackTable();

    bool find(Character::race_type race, AttackBoni &ret);

    inline bool isDataOk() {
        return m_dataOk;
    }

private:
    void reload();
    bool m_dataOk;

    void clearOldTable();

    typedef boost::unordered_map<uint16_t, AttackBoni> TABLE;
    TABLE raceAttackBoni;
};
#endif

