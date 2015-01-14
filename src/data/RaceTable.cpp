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

std::string RaceTable::getTableName() {
    return "race";
}

std::vector<std::string> RaceTable::getColumnNames() {
    return {
        "race_id",
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
        "race_attribute_points_max"
    };
}

TYPE_OF_ITEM_ID RaceTable::assignId(const Database::ResultTuple &row) {
    return uint16_t(row["race_id"].as<int32_t>());
}

RaceStruct RaceTable::assignTable(const Database::ResultTuple &row) {
    RaceStruct attributes;
    attributes.minSize = uint16_t(row["race_height_min"].as<int32_t>(100));
    attributes.maxSize = uint16_t(row["race_height_max"].as<int32_t>(100));
    attributes.minAgility = uint8_t(row["race_agility_min"].as<int32_t>(2));
    attributes.maxAgility = uint8_t(row["race_agility_max"].as<int32_t>(20));
    attributes.minConstitution = uint8_t(row["race_constitution_min"].as<int32_t>(2));
    attributes.maxConstitution = uint8_t(row["race_constitution_max"].as<int32_t>(20));
    attributes.minDexterity = uint8_t(row["race_dexterity_min"].as<int32_t>(2));
    attributes.maxDexterity = uint8_t(row["race_dexterity_max"].as<int32_t>(20));
    attributes.minEssence = uint8_t(row["race_essence_min"].as<int32_t>(2));
    attributes.maxEssence = uint8_t(row["race_essence_max"].as<int32_t>(20));
    attributes.minIntelligence = uint8_t(row["race_intelligence_min"].as<int32_t>(2));
    attributes.maxIntelligence = uint8_t(row["race_intelligence_max"].as<int32_t>(20));
    attributes.minPerception = uint8_t(row["race_perception_min"].as<int32_t>(2));
    attributes.maxPerception = uint8_t(row["race_perception_max"].as<int32_t>(20));
    attributes.minStrength = uint8_t(row["race_strength_min"].as<int32_t>(2));
    attributes.maxStrength = uint8_t(row["race_strength_max"].as<int32_t>(20));
    attributes.minWillpower = uint8_t(row["race_willpower_min"].as<int32_t>(2));
    attributes.maxWillpower = uint8_t(row["race_willpower_max"].as<int32_t>(20));
    attributes.maxAttribs = uint8_t(row["race_attribute_points_max"].as<int16_t>(84));
    return attributes;
}

uint8_t RaceTable::getRelativeSize(TYPE_OF_RACE_ID race, uint16_t size) const {
    // relative size is between 50 and 120 (in percent) and a linear interploation between min and max size
    if (!exists(race)) {
        return 100;
    }

    const auto &sizes = this->get(race);

    const auto minSize = sizes.minSize;

    const auto maxSize = sizes.maxSize;

    if (size >= minSize && size <= maxSize) {
        return uint8_t((40*(size - minSize))/(maxSize - minSize) + 80);
    } else {
        return 100;
    }
}

bool RaceTable::isBaseAttributeInLimits(TYPE_OF_RACE_ID race, Character::attributeIndex attribute, Attribute::attribute_t value) const {
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

        default: return false;
    }
}

uint8_t RaceTable::getMaxAttributePoints(TYPE_OF_RACE_ID race) const {
    if (!exists(race)) {
        return 0;
    }

    const auto &attributes = this->get(race);
    return attributes.maxAttribs;
}

