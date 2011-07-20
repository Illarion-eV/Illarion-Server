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

#include "data/WeaponObjectTable.hpp"

#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "script/LuaWeaponScript.hpp"

#include "Logger.hpp"
#include "types.hpp"

WeaponObjectTable::WeaponObjectTable() : m_dataOK(false) {
    reload();
}


void WeaponObjectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "WeaponObjectTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("weapon", "wp_itemid");
        query.addColumn("weapon", "wp_attack");
        query.addColumn("weapon", "wp_defence");
        query.addColumn("weapon", "wp_accuracy");
        query.addColumn("weapon", "wp_range");
        query.addColumn("weapon", "wp_weapontype");
        query.addColumn("weapon", "wp_ammunitiontype");
        query.addColumn("weapon", "wp_actionpoints");
        query.addColumn("weapon", "wp_magicdisturbance");
        query.addColumn("weapon", "wp_poison");
        query.addColumn("weapon", "wp_fightingscript");
        query.addServerTable("weapon");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            WeaponStruct temprecord;
            std::string scriptname;
            TYPE_OF_ITEM_ID weaponId;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                weaponId = (*itr)["wp_itemid"].as<TYPE_OF_ITEM_ID>();
                temprecord.Attack = (TYPE_OF_ATTACK)((*itr)["wp_attack"].as<uint16_t>());
                temprecord.Defence = (TYPE_OF_DEFENCE)((*itr)["wp_defence"].as<uint16_t>());
                temprecord.Accuracy = (TYPE_OF_ACCURACY)((*itr)["wp_accuracy"].as<uint16_t>());
                temprecord.Range = (TYPE_OF_RANGE)((*itr)["wp_range"].as<uint16_t>());
                temprecord.WeaponType = (TYPE_OF_WEAPONTYPE)((*itr)["wp_weapontype"].as<uint16_t>());
                temprecord.AmmunitionType = (TYPE_OF_AMMUNITIONTYPE)((*itr)["wp_ammunitiontype"].as<uint16_t>());
                temprecord.ActionPoints = (TYPE_OF_ACTIONPOINTS)((*itr)["wp_actionpoints"].as<uint16_t>());
                temprecord.MagicDisturbance = (TYPE_OF_MAGICDISTURBANCE)((*itr)["wp_magicdisturbance"].as<uint16_t>());
                temprecord.PoisonStrength = (TYPE_OF_POISONSTRENGTH)((*itr)["wp_poison"].as<uint16_t>());

                if (!(*itr)["wp_fightingscript"].is_null()) {
                    scriptname = (*itr)["wp_fightingscript"].as<std::string>();

                    if (!scriptname.empty()) {
                        try {
                            boost::shared_ptr<LuaWeaponScript> tmpScript(new LuaWeaponScript(scriptname));
                            temprecord.script = tmpScript;
                        } catch (ScriptException &e) {
                            Logger::writeError("scripts", "Error while loading weapon script: " + scriptname + ":\n" + e.what() + "\n");
                        }
                    }
                }

                m_table[weaponId] = temprecord;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into WeaponObjectTable" << std::endl;
#endif

    } catch (...) {
        m_dataOK = false;
    }

}

bool WeaponObjectTable::find(TYPE_OF_ITEM_ID Id, WeaponStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}



void WeaponObjectTable::clearOldTable() {
    m_table.clear();
}


WeaponObjectTable::~WeaponObjectTable() {
    clearOldTable();
}


