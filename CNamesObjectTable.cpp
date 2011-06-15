#include "db/ConnectionManager.h"
#include "CNamesObjectTable.h"
#include <iostream>

CNamesObjectTable::CNamesObjectTable() : m_dataOK(true) {
	reload();
}

void CNamesObjectTable::reload() {
#ifdef CDataConnect_DEBUG
	std::cout << "CNamesObjectTable: reload" << std::endl;
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
		} else m_dataOK = false;


#ifdef CDataConnect_DEBUG
		std::cout << "loaded " << rows << " rows into CNamesObjectTable" << std::endl;
#endif

	} catch (...) {
		m_dataOK = false;
	}

}

bool CNamesObjectTable::find( TYPE_OF_ITEM_ID Id, NamesStruct &ret ) {
	TABLE::iterator iterator;
	iterator = m_table.find( Id );

	if ( iterator == m_table.end() ) {
		return false;
	} else {
		ret = ( *iterator ).second;
		return true;
	}
}



void CNamesObjectTable::clearOldTable() {
	m_table.clear();
}


CNamesObjectTable::~CNamesObjectTable() {
	clearOldTable();
}



