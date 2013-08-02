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

#include "data/RaceAttributeTable.hpp"

std::string RaceAttributeTable::getTableName() {
    return "raceattr";
}

std::vector<std::string> RaceAttributeTable::getColumnNames() {
    return {
        "id",
        "minbodyheight",
        "maxbodyheight",
        "minagility",
        "maxagility",
        "minconstitution",
        "maxconstitution",
        "mindexterity",
        "maxdexterity",
        "minessence",
        "maxessence",
        "minintelligence",
        "maxintelligence",
        "minperception",
        "maxperception",
        "minstrength",
        "maxstrength",
        "minwillpower",
        "maxwillpower",
        "maxattribs"
    };
}

TYPE_OF_ITEM_ID RaceAttributeTable::assignId(const Database::ResultTuple &row) {
    return uint16_t(row["id"].as<int32_t>());
}

RaceAttributeStruct RaceAttributeTable::assignTable(const Database::ResultTuple &row) {
    RaceAttributeStruct attributes;
    attributes.minSize = uint16_t(row["minbodyheight"].as<int32_t>(100));
    attributes.maxSize = uint16_t(row["maxbodyheight"].as<int32_t>(100));
    attributes.minAgility = uint8_t(row["minagility"].as<int32_t>(2));
    attributes.maxAgility = uint8_t(row["maxagility"].as<int32_t>(20));
    attributes.minConstitution = uint8_t(row["minconstitution"].as<int32_t>(2));
    attributes.maxConstitution = uint8_t(row["maxconstitution"].as<int32_t>(20));
    attributes.minDexterity = uint8_t(row["mindexterity"].as<int32_t>(2));
    attributes.maxDexterity = uint8_t(row["maxdexterity"].as<int32_t>(20));
    attributes.minEssence = uint8_t(row["minessence"].as<int32_t>(2));
    attributes.maxEssence = uint8_t(row["maxessence"].as<int32_t>(20));
    attributes.minIntelligence = uint8_t(row["minintelligence"].as<int32_t>(2));
    attributes.maxIntelligence = uint8_t(row["maxintelligence"].as<int32_t>(20));
    attributes.minPerception = uint8_t(row["minperception"].as<int32_t>(2));
    attributes.maxPerception = uint8_t(row["maxperception"].as<int32_t>(20));
    attributes.minStrength = uint8_t(row["minstrength"].as<int32_t>(2));
    attributes.maxStrength = uint8_t(row["maxstrength"].as<int32_t>(20));
    attributes.minWillpower = uint8_t(row["minwillpower"].as<int32_t>(2));
    attributes.maxWillpower = uint8_t(row["maxwillpower"].as<int32_t>(20));
    attributes.maxAttribs = uint8_t(row["maxattribs"].as<int16_t>(84));
    return attributes;
}

uint8_t RaceAttributeTable::getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) {
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

