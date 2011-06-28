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


#ifndef COMMONOBJECTTABLE_H
#define COMMONOBJECTTABLE_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "script/LuaItemScript.hpp"

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

class World;

//! eine Tabelle für allgemeine Item-Eigenschaften
class CommonObjectTable {
	public:
		bool find( TYPE_OF_ITEM_ID Id, CommonStruct &ret );
		bool find( TYPE_OF_ITEM_ID Id);
        boost::shared_ptr<LuaItemScript> findScript( TYPE_OF_ITEM_ID Id );

		//CWorld für den Constructor aufgrund der Scriptfunktionen
		CommonObjectTable();

		~CommonObjectTable();

		void reload();

		inline bool dataOK() {
			return m_dataOK;
		}
		
	protected:

		//! der Datentyp der die Tabelle aufnimmt
		typedef hash_map < TYPE_OF_ITEM_ID, CommonStruct > TABLE;
        typedef boost::shared_ptr<LuaItemScript> iscript;
        typedef hash_map < TYPE_OF_ITEM_ID, iscript > SCRIPTTABLE;

		//! die Tabelle mit den eingelesenen Werten
		TABLE m_table;
        SCRIPTTABLE m_scripttable;

		void clearOldTable();
        TYPE_OF_ITEM_ID calcInfiniteRot( TYPE_OF_ITEM_ID id, std::map<TYPE_OF_ITEM_ID, bool> & visited, std::map<TYPE_OF_ITEM_ID, bool> & assigned );

		bool m_dataOK;

};

#endif
