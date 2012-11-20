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

#include "data/NamesObjectTable.hpp"

#include <iostream>

#include "db/SelectQuery.hpp"
#include "db/Result.hpp"

#include "TableStructs.hpp"
#include "types.hpp"

NamesObjectTable::NamesObjectTable() : m_dataOK(true) {
    reload();
}

void NamesObjectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "NamesObjectTable: reload" << std::endl;
#endif

    try {
        Database::SelectQuery query;
        query.addColumn("itemname", "itn_itemid");
        query.addColumn("itemname", "itn_german");
        query.addColumn("itemname", "itn_english");
        query.addServerTable("itemname");

        Database::Result results = query.execute();

        if (!results.empty()) {
            clearOldTable();
            NamesStruct temprecord;

            for (Database::ResultConstIterator itr = results.begin();
                 itr != results.end(); ++itr) {
                temprecord.German = (TYPE_OF_GERMAN)((*itr)["itn_german"].as<std::string>());
                temprecord.English = (TYPE_OF_ENGLISH)((*itr)["itn_english"].as<std::string>());
                m_table[(TYPE_OF_ITEM_ID)((*itr)["itn_itemid"].as<int32_t>())] = temprecord;
            }

            m_dataOK = true;
        } else {
            m_dataOK = false;
        }


#ifdef DataConnect_DEBUG
        std::cout << "loaded " << rows << " rows into NamesObjectTable" << std::endl;
#endif

    } catch (...) {
        m_dataOK = false;
    }

}

bool NamesObjectTable::find(TYPE_OF_ITEM_ID Id, NamesStruct &ret) {
    TABLE::iterator iterator;
    iterator = m_table.find(Id);

    if (iterator == m_table.end()) {
        return false;
    } else {
        ret = (*iterator).second;
        return true;
    }
}



void NamesObjectTable::clearOldTable() {
    m_table.clear();
}


NamesObjectTable::~NamesObjectTable() {
    clearOldTable();
}



