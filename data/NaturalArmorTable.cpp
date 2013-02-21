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

#include "data/NaturalArmorTable.hpp"
#include "types.hpp"

std::string NaturalArmorTable::getTableName() {
    return "naturalarmor";
}

std::vector<std::string> NaturalArmorTable::getColumnNames() {
    return {
        "nar_race",
        "nar_strokearmor",
        "nar_puncturearmor",
        "nar_thrustarmor"
    };
}

uint16_t NaturalArmorTable::assignId(const Database::ResultTuple &row) {
    return uint16_t(row["nar_race"].as<int32_t>());
}

MonsterArmor NaturalArmorTable::assignTable(const Database::ResultTuple &row) {
    MonsterArmor armor;
    armor.strokeArmor = TYPE_OF_STROKEARMOR(row["nar_strokearmor"].as<int16_t>());
    armor.punctureArmor = TYPE_OF_PUNCTUREARMOR(row["nar_puncturearmor"].as<int16_t>());
    armor.thrustArmor = TYPE_OF_THRUSTARMOR(row["nar_thrustarmor"].as<int16_t>());
    return armor;
}

