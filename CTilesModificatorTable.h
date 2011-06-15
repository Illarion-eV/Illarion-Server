#ifndef CTILESMODIFICATORTABLE_H
#define CTILESMODIFICATORTABLE_H


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

#include "constants.h"
#include "TableStructs.hpp"

//! eine Tabelle fr allgemeine Item-Eigenschaften
class CTilesModificatorTable {
	public:
		bool find( TYPE_OF_ITEM_ID Id, TilesModificatorStruct &ret );

		CTilesModificatorTable();

		~CTilesModificatorTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}

		//! prft, ob das Item mit der ID Id passierbar ist
		// \param Id eine Item-Id
		// \return true falls das Item mit der ID Id nicht passierbar ist
		bool nonPassable( TYPE_OF_ITEM_ID Id );

	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, TilesModificatorStruct > TABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;

		void clearOldTable();

		bool m_dataOK;

};

#endif



