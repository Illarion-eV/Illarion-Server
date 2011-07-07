//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.

#include "ArmorObjectTable.hpp"

#include "db/ConnectionManager.hpp"
#include "db/SchemaHelper.hpp"
#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "types.hpp"

#include <iostream>
#include <sstream>

ArmorObjectTable::ArmorObjectTable() : m_dataOK(false) {
    reload();
}

void ArmorObjectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "ArmorObjectTable: reload" << std::endl;
#endif

    try {
        Database::PConnection connection =
            Database::ConnectionManager::getInstance().getConnection();

        Database::SelectQuery query(*connection);
        query.addColumn("armor", "arm_itemid");
        query.addColumn("armor", "arm_bodyparts");
        query.addColumn("armor", "arm_puncture");
        query.addColumn("armor", "arm_stroke");
        query.addColumn("armor", "arm_thrust");
        query.addColumn("armor", "arm_magicdisturbance");
        query.addColumn("armor", "arm_absorb");
        query.addColumn("armor", "arm_stiffness");
        query.addServerTable("armor");

        Database::Result results = query.execute();
        Database::ConnectionManager::getInstance().releaseConnection(connection);

        if (!results.empty()) {
            clearOldTable();
            ArmorStruct temprecord;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {

                temprecord.BodyParts = (TYPE_OF_BODYPARTS)((*itr)["arm_bodyparts"].as<int16_t>());
                temprecord.PunctureArmor = (TYPE_OF_PUNCTUREARMOR)((*itr)["arm_puncture"].as<int16_t>());
                temprecord.StrokeArmor = (TYPE_OF_STROKEARMOR)((*itr)["arm_stroke"].as<int16_t>());
                temprecord.ThrustArmor = (TYPE_OF_THRUSTARMOR)((*itr)["arm_thrust"].as<int16_t>());
                temprecord.MagicDisturbance = (TYPE_OF_MAGICDISTURBANCE)(*itr)["arm_magicdisturbance"].as<int32_t>();
                temprecord.Absorb = (*itr)["arm_absorb"].as<int16_t>();
                temprecord.Stiffness = (*itr)["arm_stiffness"].as<int16_t>();

                m_table[(TYPE_OF_ITEM_ID)((*itr)["arm_itemid"].as<TYPE_OF_ITEM_ID>())] = temprecord;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows;
        std::cout << " rows into ArmorObjectTable" << std::endl;
#endif

    } catch (...) {
        m_dataOK = false;
    }
}

bool ArmorObjectTable::find(TYPE_OF_ITEM_ID Id, ArmorStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}



void ArmorObjectTable::clearOldTable() {
    m_table.clear();
}


ArmorObjectTable::~ArmorObjectTable() {
    clearOldTable();
}
