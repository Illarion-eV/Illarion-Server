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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 *
 * You should have received a copy of the GNU  General Public License along with
 * Illarionserver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "data/SpellTable.hpp"

std::string SpellTable::getTableName() {
    return "spells";
}

std::vector<std::string> SpellTable::getColumnNames() {
    return {
        "spl_spellid",
        "spl_magictype",
        "spl_scriptname"
    };
}

Spell SpellTable::assignId(const Database::ResultTuple &row) {
    Spell spell;
    spell.magicType = uint8_t(row["spl_magictype"].as<uint16_t>());
    spell.spellId = row["spl_spellid"].as<uint32_t>();
    return spell;
}

SpellStruct SpellTable::assignTable(const Database::ResultTuple &row) {
    return SpellStruct();
}

std::string SpellTable::assignScriptName(const Database::ResultTuple &row) {
    return row["spl_scriptname"].as<std::string>("");
}

