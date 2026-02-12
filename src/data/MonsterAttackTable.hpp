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

#ifndef MONSTER_ATTACK_TABLE
#define MONSTER_ATTACK_TABLE

#include "data/StructTable.hpp"

/**
 * @brief Attack bonus data for monster races
 *
 * Defines special attack properties that modify a monster's combat behavior
 * beyond base weapon stats.
 */
struct AttackBoni {
    uint8_t attackType = 0;         ///< Type of special attack (bite, claw, magic, etc.)
    int16_t attackValue = 0;        ///< Bonus/penalty to attack roll
    int16_t actionPointsLost = 0;   ///< Action point cost for this attack
};

/**
 * @brief Table for monster race-specific attack bonuses
 *
 * Loads monster attack data from the database "monsterattack" table, defining
 * special combat abilities and modifiers for different monster races. Each
 * race can have unique attack types with their own bonuses:
 * - Natural weapons (claws, fangs, stingers)
 * - Special attack forms (breath weapons, magical attacks)
 * - Attack roll modifiers
 * - Action point costs
 *
 * This supplements weapon data to give monsters distinctive combat styles
 * based on their race/type rather than just equipped items.
 *
 * Database table: monsterattack
 * Columns: mat_race_type, mat_attack_type, mat_attack_value, mat_actionpointslost
 */
class MonsterAttackTable : public StructTable<uint16_t, AttackBoni> {
public:
    /**
     * @brief Get database table name
     * @return "monsterattack"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the monsterattack table
     * @return Vector containing monster attack column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract race type ID from database row
     * @param row Database result row
     * @return Race type ID from mat_race_type column
     */
    auto assignId(const Database::ResultTuple &row) -> uint16_t override;

    /**
     * @brief Parse database row into AttackBoni
     * @param row Database result row
     * @return Populated AttackBoni with attack properties
     */
    auto assignTable(const Database::ResultTuple &row) -> AttackBoni override;
};

#endif
