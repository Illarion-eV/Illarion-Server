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


#include <map>
#include <TableStructs.hpp>
#include "types.hpp"
#include <vector>


class RaceTypeTable {
public:
    RaceTypeTable();

    [[nodiscard]] inline auto isDataOK() const -> bool {
        return dataOK;
    }

    [[nodiscard]] auto getRandomRaceConfiguration(TYPE_OF_RACE_ID race) const -> const RaceConfiguration;
    [[nodiscard]] auto isHairAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t hair) const -> bool;
    [[nodiscard]] auto isBeardAvailable(TYPE_OF_RACE_ID race, TYPE_OF_RACE_TYPE_ID type, uint16_t beard) const -> bool;

private:
    struct RaceTypeStruct {
        std::vector<uint16_t> hair;
        std::vector<uint16_t> beard;
        std::vector<Colour> hairColour;
        std::vector<Colour> skinColour;
    };
    using TABLE = std::map<TYPE_OF_RACE_ID, std::map<TYPE_OF_RACE_TYPE_ID, RaceTypeStruct>>;
    TABLE table;
    bool dataOK = false;
};
#endif

