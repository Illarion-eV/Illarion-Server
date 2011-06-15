#ifndef CWEAPONOBJECTTABLE_H
#define CWEAPONOBJECTTABLE_H

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

class CWorld;

//! eine Tabelle für allgemeine Item-Eigenschaften
class CWeaponObjectTable {

	public:

		bool find( TYPE_OF_ITEM_ID Id, WeaponStruct &ret );

		CWeaponObjectTable();

		~CWeaponObjectTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}

	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, WeaponStruct > TABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;

		void clearOldTable();

		bool m_dataOK;
};

#endif
