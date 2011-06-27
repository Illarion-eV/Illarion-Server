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
#include "ContainerObjectTable.hpp"
#include <iostream>

ContainerObjectTable::ContainerObjectTable() : m_dataOK(false) {
	reload();
}

void ContainerObjectTable::reload() {
#ifdef DataConnect_DEBUG
	std::cout << "ContainerObjectTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<TYPE_OF_CONTAINERVOLUME> volume;

		size_t rows = di::select_all<
					  di::Integer, di::Integer
					  >(transaction, ids, volume,
						"SELECT con_itemid, con_volume FROM container");

		if (rows > 0) {
			clearOldTable();
			ContainerStruct temprecord;
			for (size_t i = 0; i < rows; ++i) {
				temprecord.ContainerVolume = volume[i];
				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;

#ifdef DataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into ContainerObjectTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}


}

bool ContainerObjectTable::find( TYPE_OF_ITEM_ID Id, ContainerStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}


void ContainerObjectTable::clearOldTable() {
	m_table.clear();
}


ContainerObjectTable::~ContainerObjectTable() {
	clearOldTable();
}


bool ContainerObjectTable::find( TYPE_OF_ITEM_ID Id ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		return true;
	}
}
