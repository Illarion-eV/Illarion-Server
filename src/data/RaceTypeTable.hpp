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

#ifndef RACE_TYPE_TABLE_HPP
#define RACE_TYPE_TABLE_HPP

#include "types.hpp"

#include <TableStructs.hpp>
#include <map>
#include <vector>

/**
 * @brief Table for race appearance variations (subtypes)
 *
 * Loads race subtype data from multiple database tables (race_types, race_hair,
 * race_beard, race_hair_colour, race_skin_colour), defining available appearance
 * options for each race and subtype combination.
 *
 * Each race (human, dwarf, elf, etc.) can have multiple subtypes representing
 * different ethnicities or variants, each with their own sets of:
 * - Available hairstyles
 * - Available beard styles (if applicable)
 * - Available hair colors
 * - Available skin colors
 *
 * This data is used for:
 * - Character creation (offering valid appearance options)
 * - Validating character appearance
 * - Generating random NPCs with appropriate appearances
 *
 * @note Uses older direct database loading rather than StructTable
 *
 * Database tables: race_types, race_hair, race_beard, race_hair_colour, race_skin_colour
 */
class RaceTypeTable {
public:
    /**
     * @brief Constructor - loads race subtype data from database
     *
     * Queries multiple database tables to build the complete mapping of
     * race/subtype combinations to appearance options.
     */
    RaceTypeTable();

    /**
     * @brief Check if table data loaded successfully
     * @return true if data is valid, false if loading failed
     */
    [[nodiscard]] inline auto isDataOK() const -> bool { return dataOK; }

    /**
     * @brief Get a random valid appearance configuration for a race
     *
     * Selects a random subtype and generates random appearance values
     * (hairstyle, beard, colors) appropriate for that race.
     *
     * @param race Race ID to generate configuration for
     * @return Random RaceConfiguration with valid appearance options
     */
    [[nodiscard]] auto getRandomRaceConfiguration(TYPE_OF_RACE_ID race) const -> RaceConfiguration;

    /**
     * @brief Check if a hairstyle is valid for a race/subtype
     * @param race Race ID
     * @param type Subtype ID
     * @param hair Hairstyle ID
     * @return true if hairstyle is available, false otherwise
     */
    [[nodiscard]] auto isHairAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t hair) const -> bool;

    /**
     * @brief Check if a beard style is valid for a race/subtype
     * @param race Race ID
     * @param type Subtype ID
     * @param beard Beard style ID
     * @return true if beard style is available, false otherwise
     */
    [[nodiscard]] auto isBeardAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t beard) const -> bool;

private:
    /**
     * @brief Appearance options for a specific race subtype
     */
    struct RaceTypeStruct {
        std::vector<uint16_t> hair;         ///< Available hairstyle IDs
        std::vector<uint16_t> beard;        ///< Available beard style IDs
        std::vector<Colour> hairColour;     ///< Available hair colors
        std::vector<Colour> skinColour;     ///< Available skin colors
    };

    using TABLE = std::map<TYPE_OF_RACE_ID, std::map<TYPE_OF_RACE_TYPE_ID, RaceTypeStruct>>;
    TABLE table;            ///< Nested map: race -> subtype -> appearance options
    bool dataOK = false;    ///< Whether data loaded successfully
};
#endif
