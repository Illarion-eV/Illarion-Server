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

#include "data/ContainerObjectTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

ContainerObjectTable::ContainerObjectTable() : m_dataOK(false) {
    reload();
}

void ContainerObjectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "ContainerObjectTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("container", "con_itemid");
        query.addColumn("container", "con_slots");
        query.addServerTable("container");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                m_table[(TYPE_OF_ITEM_ID)((*itr)["con_itemid"].as<TYPE_OF_ITEM_ID>())]
                = (TYPE_OF_CONTAINERSLOTS)((*itr)["con_slots"].as<TYPE_OF_CONTAINERSLOTS>());
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }

#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into ContainerObjectTable" << std::endl;
#endif

    } catch (...) {
        m_dataOK = false;
    }


}

void ContainerObjectTable::clearOldTable() {
    m_table.clear();
}


ContainerObjectTable::~ContainerObjectTable() {
    clearOldTable();
}


TYPE_OF_CONTAINERSLOTS ContainerObjectTable::find(TYPE_OF_ITEM_ID id) {
    TABLE::iterator iterator;
    iterator = m_table.find(id);

    if (iterator == m_table.end()) {
        return 0;
    } else {
        return (*iterator).second;
    }
}

