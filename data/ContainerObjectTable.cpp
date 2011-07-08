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
        query.addColumn("container", "con_volume");
        query.addServerTable("container");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            ContainerStruct temprecord;

            for (Database::Result::ConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                temprecord.ContainerVolume = (TYPE_OF_VOLUME)((*itr)["con_volume"].as<int32_t>());
                m_table[(TYPE_OF_ITEM_ID)((*itr)["con_itemid"].as<TYPE_OF_ITEM_ID>())] = temprecord;
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

bool ContainerObjectTable::find(TYPE_OF_ITEM_ID Id, ContainerStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}


void ContainerObjectTable::clearOldTable() {
    m_table.clear();
}


ContainerObjectTable::~ContainerObjectTable() {
    clearOldTable();
}


bool ContainerObjectTable::find(TYPE_OF_ITEM_ID Id) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        return true;
    }
}
