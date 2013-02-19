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

#include "data/RaceSizeTable.hpp"

std::string RaceSizeTable::getTableName() {
    return "raceattr";
}

std::vector<std::string> RaceSizeTable::getColumnNames() {
    return {
        "id",
        "minbodyheight",
        "maxbodyheight"
    };
}

TYPE_OF_ITEM_ID RaceSizeTable::assignId(const Database::ResultTuple &row) {
    return uint16_t(row["id"].as<int32_t>());
}

RaceSizeStruct RaceSizeTable::assignTable(const Database::ResultTuple &row) {
    RaceSizeStruct sizes;
    sizes.minSize = uint16_t(row["minbodyheight"].as<int32_t>(100));
    sizes.maxSize = uint16_t(row["maxbodyheight"].as<int32_t>(100));
    return sizes;
}

uint8_t RaceSizeTable::getRelativeSize(Character::race_type race, uint16_t size) {
    // relative size is between 50 and 120 (in percent) and a linear interploation between min and max size
    const auto raceId = static_cast<uint16_t>(race);

    if (!exists(raceId)) {
        return 100;
    }

    const auto &sizes = (*this)[raceId];
    const auto minSize = sizes.minSize;
    const auto maxSize = sizes.maxSize;

    if (size >= minSize && size <= maxSize) {
        return uint8_t((40*(size - minSize))/(maxSize - minSize) + 80);
    } else {
        return 100;
    }
}

