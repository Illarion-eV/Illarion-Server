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

#include "RaceTable.hpp"

auto RaceTable::getTableName() -> std::string { return "race"; }

auto RaceTable::getColumnNames() -> std::vector<std::string> {
    return {"race_id",
            "race_name",
            "race_height_min",
            "race_height_max",
            "race_agility_min",
            "race_agility_max",
            "race_constitution_min",
            "race_constitution_max",
            "race_dexterity_min",
            "race_dexterity_max",
            "race_essence_min",
            "race_essence_max",
            "race_intelligence_min",
            "race_intelligence_max",
            "race_perception_min",
            "race_perception_max",
            "race_strength_min",
            "race_strength_max",
            "race_willpower_min",
            "race_willpower_max",
            "race_attribute_points_max"};
}

auto RaceTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID {
    return uint16_t(row["race_id"].as<int32_t>());
}

auto RaceTable::assignTable(const Database::ResultTuple &row) -> RaceStruct {
    RaceStruct race;
    race.serverName = row["race_name"].as<std::string>("unknown");
    race.minSize = uint16_t(row["race_height_min"].as<int32_t>(RaceStruct::defaultMinHeight));
    race.maxSize = uint16_t(row["race_height_max"].as<int32_t>(RaceStruct::defaultMaxHeight));
    race.minAgility = uint8_t(row["race_agility_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxAgility = uint8_t(row["race_agility_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minConstitution = uint8_t(row["race_constitution_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxConstitution = uint8_t(row["race_constitution_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minDexterity = uint8_t(row["race_dexterity_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxDexterity = uint8_t(row["race_dexterity_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minEssence = uint8_t(row["race_essence_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxEssence = uint8_t(row["race_essence_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minIntelligence = uint8_t(row["race_intelligence_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxIntelligence = uint8_t(row["race_intelligence_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minPerception = uint8_t(row["race_perception_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxPerception = uint8_t(row["race_perception_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minStrength = uint8_t(row["race_strength_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxStrength = uint8_t(row["race_strength_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.minWillpower = uint8_t(row["race_willpower_min"].as<int32_t>(RaceStruct::defaultMinAttribute));
    race.maxWillpower = uint8_t(row["race_willpower_max"].as<int32_t>(RaceStruct::defaultMaxAttribute));
    race.maxAttribs = uint8_t(row["race_attribute_points_max"].as<int16_t>(RaceStruct::defaultMaxAttributePoints));
    return race;
}

auto RaceTable::getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) const -> uint8_t {
    // relative size is between 40 and 120 (in percent) and a linear interpolation between min and max size
    constexpr auto defaultRelativeSize = 100;

    if (size == 0) {
        // size not set
        return defaultRelativeSize;
    }

    if (!exists(race)) {
        return defaultRelativeSize;
    }

    const auto &sizes = this->get(race);

    const auto minSize = sizes.minSize;

    const auto maxSize = sizes.maxSize;

    constexpr auto minRelativeSize = 80;
    constexpr auto maxRelativeSize = 120;

    if (size < minSize) {
        return minRelativeSize;
    }

    if (size > maxSize) {
        return maxRelativeSize;
    }

    return uint8_t(((maxRelativeSize - minRelativeSize) * (size - minSize)) / (maxSize - minSize) + minRelativeSize);
}

auto RaceTable::isBaseAttributeInLimits(TYPE_OF_RACE_ID race, Character::attributeIndex attribute,
                                        Attribute::attribute_t value) const -> bool {
    if (!exists(race)) {
        return false;
    }

    const auto &attributes = this->get(race);

    switch (attribute) {
    case Character::agility:
        return value >= attributes.minAgility && value <= attributes.maxAgility;

    case Character::constitution:
        return value >= attributes.minConstitution && value <= attributes.maxConstitution;

    case Character::dexterity:
        return value >= attributes.minDexterity && value <= attributes.maxDexterity;

    case Character::essence:
        return value >= attributes.minEssence && value <= attributes.maxEssence;

    case Character::intelligence:
        return value >= attributes.minIntelligence && value <= attributes.maxIntelligence;

    case Character::perception:
        return value >= attributes.minPerception && value <= attributes.maxPerception;

    case Character::strength:
        return value >= attributes.minStrength && value <= attributes.maxStrength;

    case Character::willpower:
        return value >= attributes.minWillpower && value <= attributes.maxWillpower;

    default:
        return false;
    }
}

auto RaceTable::getMaxAttributePoints(TYPE_OF_RACE_ID race) const -> uint8_t {
    if (!exists(race)) {
        return 0;
    }

    const auto &attributes = this->get(race);
    return attributes.maxAttribs;
}
