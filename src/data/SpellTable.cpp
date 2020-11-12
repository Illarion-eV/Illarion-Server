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

#include "data/SpellTable.hpp"

auto SpellTable::getTableName() const -> std::string { return "spells"; }

auto SpellTable::getColumnNames() -> std::vector<std::string> {
    return {"spl_spellid", "spl_magictype", "spl_scriptname"};
}

auto SpellTable::assignId(const Database::ResultTuple &row) -> Spell {
    Spell spell{};
    spell.magicType = uint8_t(row["spl_magictype"].as<uint16_t>());
    spell.spellId = row["spl_spellid"].as<uint32_t>();
    return spell;
}

auto SpellTable::assignTable(const Database::ResultTuple &row) -> SpellStruct { return SpellStruct(); }

auto SpellTable::assignScriptName(const Database::ResultTuple &row) -> std::string {
    return row["spl_scriptname"].as<std::string>("");
}
