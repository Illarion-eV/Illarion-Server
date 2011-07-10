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

#include "data/SpellTable.hpp"

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaMagicScript.hpp"

#include "Logger.hpp"

template<class from>
const std::string toString(const from &convert) {
    std::stringstream stream;
    stream << convert;
    return stream.str();
}

SpellTable::SpellTable() : _dataOK(false) {
    //im Constructor Daten Reloaden
    reload();
}

void SpellTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"SpellTable: reload!" <<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("spells", "spl_spellid");
        query.addColumn("spells", "spl_magictype");
        query.addColumn("spells", "spl_scriptname");
        query.addServerTable("spells");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            SpellStruct spell;
            uint32_t spellid;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                spellid = (uint32_t)((*itr)["spl_spellid"].as<uint32_t>());
                spell.magictype = (uint8_t)((*itr)["spl_magictype"].as<int16_t>());
                spell.scriptname = (*itr)["spl_magictype"].as<std::string>();

                if (!spell.scriptname.empty()) {
                    try {
                        boost::shared_ptr<LuaMagicScript> script(new LuaMagicScript(spell.scriptname, spellid));
                        spell.script = script;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading script: " + spell.scriptname + ":\n" + e.what() + "\n");
                    }
                }

                Spells[spellid] = spell;
            }
        }

        std::cout << " loadet magic Scripts! " << std::endl;
        _dataOK = true;
    } catch (std::exception &e) {

        std::cerr << "exception: " << e.what() << std::endl;
        _dataOK = false;
    }
}

bool SpellTable::find(unsigned long int magicFlag,unsigned short int magic_type, SpellStruct &magicSpell) {
    std::pair<SpellMap::iterator, SpellMap::iterator> range = Spells.equal_range(magicFlag);
    SpellMap::iterator iter;

    //Liste durchlaufen und prüfen ob der Magictype stimmt.
    for (iter = range.first; iter != range.second; ++iter) {
        if ((*iter).second.magictype == magic_type) {
            magicSpell = (*iter).second;
            return true;
        }
    }

    return false;
}

void SpellTable::clearOldTable() {
    Spells.clear();
}

SpellTable::~SpellTable() {
    clearOldTable();
}


