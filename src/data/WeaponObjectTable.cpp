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

#include "data/WeaponObjectTable.hpp"

auto WeaponObjectTable::getTableName() -> std::string { return "weapon"; }

auto WeaponObjectTable::getColumnNames() -> std::vector<std::string> {
    return {"wp_itemid",        "wp_attack",         "wp_defence",      "wp_accuracy",         "wp_range",
            "wp_weapontype",    "wp_ammunitiontype", "wp_actionpoints", "wp_magicdisturbance", "wp_poison",
            "wp_fightingscript"};
}

auto WeaponObjectTable::assignId(const Database::ResultTuple &row) -> TYPE_OF_ITEM_ID {
    return row["wp_itemid"].as<TYPE_OF_ITEM_ID>();
}

auto WeaponObjectTable::assignTable(const Database::ResultTuple &row) -> WeaponStruct {
    WeaponStruct weapon;
    weapon.Attack = TYPE_OF_ATTACK(row["wp_attack"].as<uint16_t>());
    weapon.Defence = TYPE_OF_DEFENCE(row["wp_defence"].as<uint16_t>());
    weapon.Accuracy = TYPE_OF_ACCURACY(row["wp_accuracy"].as<uint16_t>());
    weapon.Range = TYPE_OF_RANGE(row["wp_range"].as<uint16_t>());
    weapon.Type = TYPE_OF_WEAPONTYPE(row["wp_weapontype"].as<uint16_t>());
    weapon.AmmunitionType = TYPE_OF_AMMUNITIONTYPE(row["wp_ammunitiontype"].as<uint16_t>());
    weapon.ActionPoints = TYPE_OF_ACTIONPOINTS(row["wp_actionpoints"].as<uint16_t>());
    weapon.MagicDisturbance = TYPE_OF_MAGICDISTURBANCE(row["wp_magicdisturbance"].as<uint16_t>());
    weapon.PoisonStrength = TYPE_OF_POISONSTRENGTH(row["wp_poison"].as<uint16_t>());
    return weapon;
}

auto WeaponObjectTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["wp_fightingscript"].as<std::string>("");
}
