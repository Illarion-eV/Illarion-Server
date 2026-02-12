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

#ifndef NATURAL_ARMOR_TABLE_HPP
#define NATURAL_ARMOR_TABLE_HPP

#include "TableStructs.hpp"
#include "data/StructTable.hpp"

/**
 * @brief Table for race-based natural armor values
 *
 * Loads natural armor data from the database "naturalarmor" table, defining
 * the innate defensive properties of different races. Natural armor represents
 * physical resistance that comes from a race's body (thick skin, scales, fur,
 * exoskeleton, etc.) rather than worn equipment.
 *
 * Each race has armor values against three damage types:
 * - Stroke armor (defense against slashing attacks)
 * - Puncture armor (defense against piercing attacks)
 * - Thrust armor (defense against thrusting attacks)
 *
 * These values are primarily used for monsters but can apply to any race.
 * Natural armor stacks with or modifies worn armor depending on game rules.
 *
 * Database table: naturalarmor
 * Columns: nar_race, nar_strokearmor, nar_puncturearmor, nar_thrustarmor
 */
class NaturalArmorTable : public StructTable<uint16_t, MonsterArmor> {
public:
    /**
     * @brief Get database table name
     * @return "naturalarmor"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the naturalarmor table
     * @return Vector containing natural armor column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract race ID from database row
     * @param row Database result row
     * @return Race ID from nar_race column
     */
    auto assignId(const Database::ResultTuple &row) -> uint16_t override;

    /**
     * @brief Parse database row into MonsterArmor
     * @param row Database result row
     * @return Populated MonsterArmor with armor values for all damage types
     */
    auto assignTable(const Database::ResultTuple &row) -> MonsterArmor override;
};

#endif
