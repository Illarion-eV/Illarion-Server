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

#ifndef SKILL_TABLE_HPP
#define SKILL_TABLE_HPP

#include "TableStructs.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"

/**
 * @brief Table for skill definitions and localized names
 *
 * Loads skill data from the database "skills" table, providing access to
 * skill IDs and their localized names. Skills represent character abilities
 * such as crafting, combat, and magic skills.
 *
 * Database columns: skl_skill_id, skl_name, skl_name_english, skl_name_german
 */
class SkillTable : public StructTable<TYPE_OF_SKILL_ID, SkillStruct> {
public:
    /**
     * @brief Get database table name
     * @return "skills"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the skills table
     * @return Vector containing skill table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract skill ID from database row
     * @param row Database result row
     * @return Skill ID from skl_skill_id column
     */
    auto assignId(const Database::ResultTuple &row) -> TYPE_OF_SKILL_ID override;

    /**
     * @brief Parse database row into SkillStruct
     * @param row Database result row
     * @return Populated SkillStruct with server name and localized names
     */
    auto assignTable(const Database::ResultTuple &row) -> SkillStruct override;
};

#endif
