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

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "types.hpp"

NaturalArmorTable::NaturalArmorTable() : m_dataOK(false) {
    reload();
}

NaturalArmorTable::~NaturalArmorTable() {
    clearOldTable();
}

void NaturalArmorTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout<<"Trying to reload NaturalArmorTable!"<<std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("naturalarmor", "nar_race");
        query.addColumn("naturalarmor", "nar_strokeArmor");
        query.addColumn("naturalarmor", "nar_punctureArmor");
        query.addColumn("naturalarmor", "nar_thrustArmor");
        query.addServerTable("naturalarmor");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            MonsterArmor armor;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                armor.strokeArmor = (TYPE_OF_STROKEARMOR)((*itr)["nar_strokeArmor"].as<int16_t>());
                armor.punctureArmor = (TYPE_OF_PUNCTUREARMOR)((*itr)["nar_punctureArmor"].as<int16_t>());
                armor.thrustArmor = (TYPE_OF_THRUSTARMOR)((*itr)["arm_thrust"].as<int16_t>());
                m_ArmorTable[(uint16_t)((*itr)["nar_strokeArmor"].as<int32_t>())] = armor;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into NaturalArmorTable" << std::endl;
#endif
    } catch (...) {
        m_dataOK = false;
    }

}

bool NaturalArmorTable::find(Character::race_type race, MonsterArmor &ret) {
    TABLE::iterator iterat;
    iterat = m_ArmorTable.find(static_cast<uint16_t>(race));

    if (iterat == m_ArmorTable.end()) {
        return false;
    } else {
        ret = (*iterat).second;
        return true;
    }

    return false;
}

void NaturalArmorTable::clearOldTable() {
    m_ArmorTable.clear();
}


