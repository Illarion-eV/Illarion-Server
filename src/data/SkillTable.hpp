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

#ifndef _SKILL_TABLE_HPP_
#define _SKILL_TABLE_HPP_

#include "data/StructTable.hpp"
#include "types.hpp"
#include "TableStructs.hpp"

class SkillTable : public StructTable<TYPE_OF_SKILL_ID, SkillStruct> {
public:
    std::string getTableName() override;
    std::vector<std::string> getColumnNames() override;
    TYPE_OF_SKILL_ID assignId(const Database::ResultTuple &row) override;
    SkillStruct assignTable(const Database::ResultTuple &row) override;
};

#endif

