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

#ifndef MONSTER_TABLE_HPP
#define MONSTER_TABLE_HPP

#include "script/LuaMonsterScript.hpp"

#include <TableStructs.hpp>
#include <boost/unordered_map.hpp>

class MonsterTable {
public:
    MonsterTable();

    [[nodiscard]] inline auto isDataOK() const -> bool {
        return dataOK;
    }

    [[nodiscard]] auto exists(TYPE_OF_CHARACTER_ID id) const -> bool;
    auto operator[](TYPE_OF_CHARACTER_ID id) -> const MonsterStruct &;

private:
    using TABLE = boost::unordered_map<TYPE_OF_CHARACTER_ID, MonsterStruct>;
    TABLE table;
    bool dataOK = false;
};
#endif
