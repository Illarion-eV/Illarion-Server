#include "db/ConnectionManager.h"
#include "CContainerObjectTable.h"
#include <iostream>

CContainerObjectTable::CContainerObjectTable() : m_dataOK(false) {
	reload();
}

void CContainerObjectTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CContainerObjectTable: reload" << std::endl;
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

#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CContainerObjectTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}


}

bool CContainerObjectTable::find( TYPE_OF_ITEM_ID Id, ContainerStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}


void CContainerObjectTable::clearOldTable() {
	m_table.clear();
}


CContainerObjectTable::~CContainerObjectTable() {
	clearOldTable();
}


bool CContainerObjectTable::find( TYPE_OF_ITEM_ID Id ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		return true;
	}
}
