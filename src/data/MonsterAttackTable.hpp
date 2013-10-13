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

#ifndef _MONSTER_ATTACK_TABLE_
#define _MONSTER_ATTACK_TABLE_

#include "data/StructTable.hpp"

struct AttackBoni {
    uint8_t attackType = 0;
    int16_t attackValue = 0;
    int16_t actionPointsLost = 0;
};

class MonsterAttackTable : public StructTable<uint16_t, AttackBoni> {
public:
    virtual std::string getTableName() override;
    virtual std::vector<std::string> getColumnNames() override;
    virtual uint16_t assignId(const Database::ResultTuple &row) override;
    virtual AttackBoni assignTable(const Database::ResultTuple &row) override;
};

#endif

