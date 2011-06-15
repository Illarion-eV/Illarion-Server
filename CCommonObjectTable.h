#ifndef CCOMMONOBJECTTABLE_H
#define CCOMMONOBJECTTABLE_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "script/CLuaItemScript.hpp"

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
class CCommonObjectTable {
	public:
		bool find( TYPE_OF_ITEM_ID Id, CommonStruct &ret );
		bool find( TYPE_OF_ITEM_ID Id);
        boost::shared_ptr<CLuaItemScript> findScript( TYPE_OF_ITEM_ID Id );

		//CWorld für den Constructor aufgrund der Scriptfunktionen
		CCommonObjectTable();

		~CCommonObjectTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}
		
	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, CommonStruct > TABLE;
        typedef boost::shared_ptr<CLuaItemScript> iscript;
        typedef hash_map < TYPE_OF_ITEM_ID, iscript > SCRIPTTABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;
        SCRIPTTABLE m_scripttable;

		void clearOldTable();
        TYPE_OF_ITEM_ID calcInfiniteRot( TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> & visited, std::map<TYPE_OF_ITEM_ID, bool> & assigned );

		bool m_dataOK;

};

#endif
