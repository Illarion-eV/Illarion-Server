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
/*
 * TODO: The handling of the tile modificators is extremly unsafe. Errors in the
 *       database will corrupt the data easily und uncontrolable.
 */

#include "data/TilesModificatorTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "types.hpp"

TilesModificatorTable::TilesModificatorTable() : m_dataOK(false) {
    reload();
}

void TilesModificatorTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "TilesModificatorTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("tilesmodificators", "tim_itemid");
        query.addColumn("tilesmodificators", "tim_isnotpassable");
        query.addColumn("tilesmodificators", "tim_isnottransparent");
        query.addColumn("tilesmodificators", "tim_isnotpenetrateable");
        query.addColumn("tilesmodificators", "tim_specialitem");
        query.addColumn("tilesmodificators", "tim_groundlevel");
        query.addColumn("tilesmodificators", "tim_makepassable");
        query.addServerTable("tilesmodificators");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            TilesModificatorStruct temprecord;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                temprecord.Modificator = (uint8_t)((*itr)["tim_groundlevel"].as<uint16_t>());
                temprecord.Modificator |= (*itr)["tim_isnotpassable"].as<bool>() ? FLAG_PASSABLE : 0;
                temprecord.Modificator |= (*itr)["tim_isnottransparent"].as<bool>() ? FLAG_TRANSPARENT : 0;
                temprecord.Modificator |= (*itr)["tim_isnotpenetrateable"].as<bool>() ? FLAG_TRANSPARENT : 0;
                temprecord.Modificator |= (*itr)["tim_specialitem"].as<bool>() ? FLAG_SPECIALITEM : 0;
                temprecord.Modificator |= (*itr)["tim_makepassable"].as<bool>() ? FLAG_MAKEPASSABLE : 0;

                m_table[(*itr)["tim_itemid"].as<TYPE_OF_ITEM_ID>()] = temprecord;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into TilesModificatorTable" << std::endl;
#endif

    } catch (...) {
        m_dataOK = false;
    }

}

bool TilesModificatorTable::find(TYPE_OF_ITEM_ID Id, TilesModificatorStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}

void TilesModificatorTable::clearOldTable() {
    m_table.clear();
}

TilesModificatorTable::~TilesModificatorTable() {
    clearOldTable();
}

bool TilesModificatorTable::nonPassable(TYPE_OF_ITEM_ID Id) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        return (((*iterator).second.Modificator & FLAG_PASSABLE) != 0) && (((*iterator).second.Modificator & FLAG_MAKEPASSABLE) == 0);
    }
}
