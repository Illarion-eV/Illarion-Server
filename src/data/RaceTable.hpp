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

#ifndef RACE_ATTRIBUTE_TABLE_HPP
#define RACE_ATTRIBUTE_TABLE_HPP

#include "Character.hpp"
#include "data/StructTable.hpp"
#include "types.hpp"

/**
 * @brief Table for character race definitions and attribute limits
 *
 * Loads race data from the database "race" table, defining playable races
 * with their physical characteristics (height) and attribute constraints.
 * Each race has minimum and maximum values for all character attributes,
 * used for character creation validation.
 *
 * Provides helper methods for:
 * - Calculating relative size scaling (80-120% based on height)
 * - Validating attributes are within racial limits
 * - Retrieving maximum attribute points for character creation
 */
class RaceTable : public StructTable<uint16_t, RaceStruct> {
public:
    /**
     * @brief Get database table name
     * @return "race"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the race table
     * @return Vector containing race table column names (race_id, race_name, height ranges, attribute ranges)
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract race ID from database row
     * @param row Database result row
     * @return Race ID from race_id column
     */
    auto assignId(const Database::ResultTuple &row) -> uint16_t override;

    /**
     * @brief Parse database row into RaceStruct
     * @param row Database result row
     * @return Populated RaceStruct with name, size, and attribute limits
     */
    auto assignTable(const Database::ResultTuple &row) -> RaceStruct override;

    /**
     * @brief Calculate relative size percentage for a race and height
     *
     * Converts absolute height to relative size scaling (80-120%) using
     * linear interpolation between the race's min and max height.
     *
     * @param race Race ID to check
     * @param size Absolute character height/size
     * @return Relative size percentage (80-120), or 100 if race not found or size is 0
     */
    auto getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) const -> uint8_t;

    /**
     * @brief Check if attribute value is within racial limits
     *
     * Validates that the given attribute value falls within the min/max
     * range defined for the race.
     *
     * @param race Race ID to check
     * @param attribute Which attribute to validate (agility, strength, etc.)
     * @param value Attribute value to check
     * @return true if value is within limits, false otherwise
     */
    auto isBaseAttributeInLimits(TYPE_OF_RACE_ID race, Character::attributeIndex attribute,
                                 Attribute::attribute_t value) const -> bool;

    /**
     * @brief Get maximum attribute points for character creation
     *
     * Returns the total number of attribute points a character of this race
     * can distribute during character creation.
     *
     * @param race Race ID to check
     * @return Maximum attribute points, or 0 if race not found
     */
    auto getMaxAttributePoints(TYPE_OF_RACE_ID race) const -> uint8_t;
};

#endif
