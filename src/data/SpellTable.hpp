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

#ifndef SPELL_TABLE_HPP
#define SPELL_TABLE_HPP

#include "data/ScriptStructTable.hpp"
#include "script/LuaMagicScript.hpp"

#include <boost/functional/hash/hash.hpp>

/**
 * @brief Unique identifier for a spell
 *
 * Spells are identified by a composite key of magic type and spell ID,
 * allowing different magic systems to have overlapping spell IDs.
 */
struct Spell {
    uint8_t magicType;   ///< Type of magic (e.g., druid, priest, mage)
    uint32_t spellId;    ///< Spell identifier within the magic type

    /**
     * @brief Equality comparison for spell lookup
     * @param spell Spell to compare with
     * @return true if both magic type and spell ID match
     */
    auto operator==(const Spell &spell) const -> bool {
        return (magicType == spell.magicType && spellId == spell.spellId);
    }

    /**
     * @brief Stream output operator for debugging
     * @param out Output stream
     * @param spell Spell to output
     * @return Reference to output stream
     */
    friend auto operator<<(std::ostream &out, const Spell &spell) -> std::ostream & {
        out << "magic: " << spell.magicType << ", spell: " << spell.spellId;
        return out;
    }
};

namespace std {
/**
 * @brief Hash function specialization for Spell
 *
 * Enables use of Spell as a key in unordered_map by combining
 * hashes of magic type and spell ID.
 */
template <> struct hash<Spell> {
    auto operator()(const Spell &s) const -> size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, s.magicType);
        boost::hash_combine(seed, s.spellId);
        return seed;
    }
};
} // namespace std

/**
 * @brief Empty struct for spell data (all data is in scripts)
 *
 * Currently, spell properties are entirely handled by Lua scripts,
 * so no additional data structure is needed.
 */
struct SpellStruct {};

/**
 * @brief Table for spell definitions and magic scripts
 *
 * Loads spell data from the database "spells" table, mapping spell identifiers
 * to their associated Lua magic scripts. Each spell is uniquely identified by
 * a combination of magic type and spell ID.
 *
 * All spell behavior (casting requirements, effects, mana cost, etc.) is
 * implemented in the Lua scripts rather than in database columns.
 *
 * Database table: spells
 * Columns: spl_magictype, spl_spellid, spl_scriptname
 * Script type: LuaMagicScript
 */
class SpellTable : public ScriptStructTable<Spell, SpellStruct, LuaMagicScript> {
public:
    /**
     * @brief Get database table name
     * @return "spells"
     */
    auto getTableName() const -> std::string override;

    /**
     * @brief Get column names for the spells table
     * @return Vector containing spell table column names
     */
    auto getColumnNames() -> std::vector<std::string> override;

    /**
     * @brief Extract spell identifier from database row
     * @param row Database result row
     * @return Spell struct with magic type and spell ID
     */
    auto assignId(const Database::ResultTuple &row) -> Spell override;

    /**
     * @brief Parse database row into SpellStruct
     * @param row Database result row
     * @return Empty SpellStruct (no data stored)
     */
    auto assignTable(const Database::ResultTuple &row) -> SpellStruct override;

    /**
     * @brief Extract script filename from database row
     * @param row Database result row
     * @return Script filename from spl_scriptname column
     */
    auto assignScriptName(const Database::ResultTuple &row) -> std::string override;
};

#endif
