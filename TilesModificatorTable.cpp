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
#include "TilesModificatorTable.hpp"
#include <iostream>

CTilesModificatorTable::CTilesModificatorTable() : m_dataOK(false) {
	reload();
}

void CTilesModificatorTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CTilesModificatorTable: reload" << std::endl;
#endif

	try {
		ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();

		std::vector<TYPE_OF_ITEM_ID> ids;
		std::vector<uint8_t> notpassable;
		std::vector<uint8_t> nottransparent;
		std::vector<uint8_t> notpenetrateable;
		std::vector<uint8_t> specialitem;
		std::vector<uint8_t> groundlevel;
		std::vector<uint8_t> makepassable;

		size_t rows = di::select_all<
					  di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer, di::Integer
					  >(transaction, ids, notpassable, nottransparent, notpenetrateable, specialitem, groundlevel, makepassable,
						"SELECT tim_itemid, tim_isnotpassable, tim_isnottransparent, tim_isnotpenetrateable,"
						"tim_specialitem, tim_groundlevel, tim_makepassable FROM tilesmodificators");

		if (rows > 0) {
			clearOldTable();
			TilesModificatorStruct temprecord;
			for (size_t i = 0; i < rows; ++i) {
				temprecord.Modificator = groundlevel[i];
				temprecord.Modificator |= makepassable[i] ? FLAG_MAKEPASSABLE : 0;
				temprecord.Modificator |= notpassable[i] ? FLAG_PASSABLE : 0;
				temprecord.Modificator |= nottransparent[i] ? FLAG_TRANSPARENT : 0;
				temprecord.Modificator |= notpenetrateable[i] ? FLAG_TRANSPARENT : 0;
				temprecord.Modificator |= specialitem[i]? FLAG_SPECIALITEM : 0;

				m_table[ ids[i] ] = temprecord;
			}
			m_dataOK = true;
		} else m_dataOK = false;


#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CTilesModificatorTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}

}

bool CTilesModificatorTable::find( TYPE_OF_ITEM_ID Id, TilesModificatorStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}

void CTilesModificatorTable::clearOldTable() {
	m_table.clear();
}

CTilesModificatorTable::~CTilesModificatorTable() {
	clearOldTable();
}

bool CTilesModificatorTable::nonPassable( TYPE_OF_ITEM_ID Id ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		return ( ( ( *iterator ).second.Modificator & FLAG_PASSABLE ) != 0 ) && ( ( ( *iterator ).second.Modificator & FLAG_MAKEPASSABLE ) == 0 );
	}
}
