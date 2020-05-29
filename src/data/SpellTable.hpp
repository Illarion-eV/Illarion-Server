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

#ifndef _SPELL_TABLE_HPP_
#define _SPELL_TABLE_HPP_

#include <boost/functional/hash/hash.hpp>

#include "data/ScriptStructTable.hpp"

#include "script/LuaMagicScript.hpp"

struct Spell {
    uint8_t magicType;
    uint32_t spellId;

    auto operator==(const Spell &spell) const -> bool {
        return (magicType == spell.magicType && spellId == spell.spellId);
    }

    friend auto operator<<(std::ostream &out, const Spell &spell) -> std::ostream & {
        out << "magic: " << spell.magicType << ", spell: " << spell.spellId;
        return out;
    }
};

namespace std {
template<> struct hash<Spell> {
    auto operator()(const Spell &s) const -> size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, s.magicType);
        boost::hash_combine(seed, s.spellId);
        return seed;
    }
};
}

struct SpellStruct {
};

class SpellTable : public ScriptStructTable<Spell, SpellStruct, LuaMagicScript> {
public:
    auto getTableName() -> std::string override;
    auto getColumnNames() -> std::vector<std::string> override;
    auto assignId(const Database::ResultTuple &row) -> Spell override;
    auto assignTable(const Database::ResultTuple &row) -> SpellStruct override;
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif

