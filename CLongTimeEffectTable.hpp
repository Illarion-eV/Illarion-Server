#ifndef CLONGTIMEEFFEKTTABLE_HPP_
#define CLONGTIMEEFFEKTTABLE_HPP_

#include <string>

#include "TableStructs.hpp"

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

class CDBConnect;
class CWorld;
class CCharcter;

/**
* a table for long time effects which can have effects on each characters
*/
class CLongTimeEffectTable {
	public:
    
        bool find(uint16_t effectId, LongTimeEffectStruct &ret);
        bool find(std::string effectname, LongTimeEffectStruct &ret);
    
		CLongTimeEffectTable();

		~CLongTimeEffectTable();

		void reload();

		inline bool dataOK() 
        {
			return m_dataOK;
		}

	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < uint16_t, LongTimeEffectStruct > TABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;

		void clearOldTable();

		bool m_dataOK;

};
#endif
