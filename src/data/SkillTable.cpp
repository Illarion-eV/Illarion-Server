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

#include "data/SkillTable.hpp"

auto SkillTable::getTableName() -> std::string { return "skills"; }

auto SkillTable::getColumnNames() -> std::vector<std::string> {
    return {"skl_skill_id", "skl_name", "skl_name_english", "skl_name_german"};
}

auto SkillTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_SKILL_ID {
    return TYPE_OF_SKILL_ID(row["skl_skill_id"].as<uint16_t>());
}

auto SkillTable::assignTable(const Database::ResultTuple &row) -> SkillStruct {
    SkillStruct skill;
    skill.serverName = row["skl_name"].as<std::string>();
    skill.englishName = row["skl_name_english"].as<std::string>();
    skill.germanName = row["skl_name_german"].as<std::string>();
    return skill;
}
