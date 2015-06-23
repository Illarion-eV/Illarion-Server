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

#include "data/ArmorObjectTable.hpp"

std::string ArmorObjectTable::getTableName() {
    return "armor";
}

std::vector<std::string> ArmorObjectTable::getColumnNames() {
    return {
        "arm_itemid",
        "arm_bodyparts",
        "arm_puncture",
        "arm_stroke",
        "arm_thrust",
        "arm_magicdisturbance",
        "arm_absorb",
        "arm_stiffness",
        "arm_type"
    };
}

TYPE_OF_ITEM_ID ArmorObjectTable::assignId(const Database::ResultTuple &row) {
    return row["arm_itemid"].as<TYPE_OF_ITEM_ID>();
}

ArmorStruct ArmorObjectTable::assignTable(const Database::ResultTuple &row) {
    ArmorStruct armor;
    armor.BodyParts = TYPE_OF_BODYPARTS(row["arm_bodyparts"].as<int16_t>());
    armor.PunctureArmor = TYPE_OF_PUNCTUREARMOR(row["arm_puncture"].as<int16_t>());
    armor.StrokeArmor = TYPE_OF_STROKEARMOR(row["arm_stroke"].as<int16_t>());
    armor.ThrustArmor = TYPE_OF_THRUSTARMOR(row["arm_thrust"].as<int16_t>());
    armor.MagicDisturbance = TYPE_OF_MAGICDISTURBANCE(row["arm_magicdisturbance"].as<int32_t>());
    armor.Absorb = row["arm_absorb"].as<int16_t>();
    armor.Stiffness = row["arm_stiffness"].as<int16_t>();
    armor.Type = TYPE_OF_ARMORTYPE(row["arm_type"].as<int16_t>());
    return armor;
}

