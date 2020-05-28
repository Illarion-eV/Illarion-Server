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

#ifndef _MONSTER_TABLE_HPP_
#define _MONSTER_TABLE_HPP_

#include <boost/unordered_map.hpp>
#include <TableStructs.hpp>
#include "script/LuaMonsterScript.hpp"


class MonsterTable {
public:
    MonsterTable();

    inline bool isDataOK() const {
        return dataOK;
    }

    bool exists(TYPE_OF_CHARACTER_ID id) const;
    const MonsterStruct &operator[](TYPE_OF_CHARACTER_ID id);

private:
    using TABLE = boost::unordered_map<TYPE_OF_CHARACTER_ID, MonsterStruct>;
    TABLE table;
    bool dataOK = false;
};
#endif

