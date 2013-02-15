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

#ifndef _SKILL_TABLE_HPP_
#define _SKILL_TABLE_HPP_

#include <string>
#include <boost/unordered_map.hpp>
#include "TableStructs.hpp"
#include "types.hpp"

class World;

class SkillTable {
public:
    typedef boost::unordered_map<TYPE_OF_SKILL_ID, SkillStruct> TABLE;

    bool find(TYPE_OF_SKILL_ID Id) const;
    bool find(TYPE_OF_SKILL_ID Id, SkillStruct &ret) const;

    TABLE::const_iterator begin() const;
    TABLE::const_iterator end() const;

    SkillTable();
    ~SkillTable();

    inline bool dataOK() {
        return m_dataOK;
    }

private:
    void reload();

    TABLE m_table;

    void clearOldTable();

    bool m_dataOK;
};
#endif

