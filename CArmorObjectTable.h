#ifndef CARMOROBJECTTABLE_H
#define CARMOROBJECTTABLE_H

#include <string>

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
#endif

#endif

#include "TableStructs.hpp"

//! eine Tabelle für allgemeine Item-Eigenschaften
class CArmorObjectTable {
	public:
		bool find( TYPE_OF_ITEM_ID Id, ArmorStruct &ret );

		CArmorObjectTable();

		~CArmorObjectTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}

	protected:
		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, ArmorStruct > TABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;

		void clearOldTable();

		bool m_dataOK;

};

#endif
