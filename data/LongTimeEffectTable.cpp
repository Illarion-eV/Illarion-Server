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

#include "data/LongTimeEffectTable.hpp"

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaLongTimeEffectScript.hpp"

#include "TableStructs.hpp"
#include "Logger.hpp"

LongTimeEffectTable::LongTimeEffectTable() : m_dataOK(false) {
    reload();
}

void LongTimeEffectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "LongTimeEffectTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("longtimeeffects", "lte_effectid");
        query.addColumn("longtimeeffects", "lte_effectname");
        query.addColumn("longtimeeffects", "lte_scriptname");
        query.addServerTable("longtimeeffects");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            LongTimeEffectStruct temp;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                temp.effectid = (uint16_t)((*itr)["lte_effectid"].as<int32_t>());
                temp.effectname = (std::string)((*itr)["lte_effectid"].as<std::string>());

                if (!(*itr)["lte_scriptname"].is_null()) {
                    temp.scriptname = (std::string)((*itr)["lte_scriptname"].as<std::string>());

                    try {
                        boost::shared_ptr<LuaLongTimeEffectScript> script(new LuaLongTimeEffectScript(temp.scriptname, temp));
                        temp.script = script;
                    } catch (ScriptException &e) {
                        Logger::writeError("scripts", "Error while loading lte script: " + temp.scriptname + ":\n" + e.what() + "\n");
                    }
                }

                m_table[temp.effectid] = temp;
            }

            m_dataOK = true;
        }

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into LongTimeEffectTable" << std::endl;
#endif
    } catch (std::exception &e) {
        std::cerr<<"exception in LongTimeEffect loading: " << e.what() << std::endl;
        m_dataOK = false;
    }
}

bool LongTimeEffectTable::find(uint16_t effectId, LongTimeEffectStruct &ret) {
    TABLE::iterator it = m_table.find(effectId);

    if (it != m_table.end()) {
        ret = it->second;
        return true;
    } else {
        return false;
    }
}

bool LongTimeEffectTable::find(std::string effectname, LongTimeEffectStruct &ret) {
    for (TABLE::iterator it = m_table.begin(); it != m_table.end(); ++it) {
        if (it->second.scriptname == effectname) {
            ret = it->second;
            return true;
        }
    }

    return false;
}

void LongTimeEffectTable::clearOldTable() {
    m_table.clear();
}

LongTimeEffectTable::~LongTimeEffectTable() {
    clearOldTable();
}
