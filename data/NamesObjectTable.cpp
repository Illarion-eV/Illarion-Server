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


#include "db/ConnectionManager.hpp"
#include "NamesObjectTable.hpp"
#include <iostream>

NamesObjectTable::NamesObjectTable() : m_dataOK(true) {
    reload();
}

void NamesObjectTable::reload() {
#ifdef DataConnect_DEBUG
    std::cout << "NamesObjectTable: reload" << std::endl;
#endif

    try {
        ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

        std::vector<TYPE_OF_ITEM_ID> ids;
        std::vector<std::string> languages[3];

        size_t rows = di::select_all<
                      di::Integer, di::Varchar, di::Varchar, di::Varchar
                      >(transaction, ids, languages[0], languages[1], languages[2],
                        "SELECT itn_itemid, itn_german, itn_english, itn_french FROM itemname");

        if (rows > 0) {
            clearOldTable();
            NamesStruct temprecord;

            for (size_t i = 0; i < rows; ++i) {
                temprecord.German = languages[0][i];
                temprecord.English = languages[1][i];
                temprecord.French = languages[2][i];
                m_table[ ids[i] ] = temprecord;
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



