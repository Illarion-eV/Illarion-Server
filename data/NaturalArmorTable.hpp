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

#ifndef _NATURAL_ARMOR_TABLE_HPP_
#define _NATURAL_ARMOR_TABLE_HPP_

#include <string>
#include <boost/unordered_map.hpp>
#include "data/Table.hpp"
#include "Character.hpp"

class NaturalArmorTable: public Table {
public:
    bool find(Character::race_type race, MonsterArmor &ret);

    NaturalArmorTable();
    ~NaturalArmorTable();

    inline bool isDataOk() {
        return m_dataOK;
    }

private:

    typedef boost::unordered_map<uint16_t, MonsterArmor> TABLE;
    TABLE m_ArmorTable;

    virtual void reload();
    bool m_dataOK;

    void clearOldTable();
};
#endif

