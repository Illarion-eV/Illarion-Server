#ifndef CTILESTABLE_H
#define CTILESTABLE_H

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

#include "types.h"
#include "TableStructs.hpp"

class CDBConnect;
class CWorld;

//! eine Tabelle für allgemeine Item-Eigenschaften
class CTilesTable {
	public:
		bool find( TYPE_OF_ITEM_ID Id, TilesStruct &ret );

		CTilesTable();

		~CTilesTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}

		//! prüft, ob das Item mit der ID Id passierbar ist
		// \param Id eine Item-Id
		// \return true falls das Item mit der ID Id nicht passierbar ist
		bool nonPassable( TYPE_OF_ITEM_ID Id );

	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, TilesStruct > TABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;

		void clearOldTable();

		bool m_dataOK;

};
#endif
