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
