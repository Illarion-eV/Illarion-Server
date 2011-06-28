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


#ifndef TRIGGERTABLE_HPP
#define TRIGGERTABLE_HPP

#include <map>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <list>

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_map;
using __gnu_cxx::hash;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_map;
using std::hash;
#endif

#endif

#include "globals.hpp"
#include "script/LuaTriggerScript.hpp"

class World;

struct TriggerStruct {
	//Position des Triggerfelds
	position pos;
	//Name des Scriptes;
	std::string scriptname;
	//Zeiger auf das Script
	boost::shared_ptr<LuaTriggerScript> script;
};



class TriggerTable {
	public:

		TriggerTable();
		~TriggerTable();

		void reload();

		inline bool isDataOK() { return _dataOK; }

		bool find(position pos, TriggerStruct &data);

	protected:

		//! Vergleichsfunktion für position
		struct eqpos {
			bool operator()( position a, position b ) const {
				return ( ( a.x == b.x ) && ( a.y == b.y ) && ( a.z == b.z ) );
			}
		};

		//! Hashfunktion für position
		struct poshash {
			hash < int > inthash;
			int operator()( const position a ) const {
				int temp = ( a.x * 1000 + a.y ) * 1000 + a.z;
				return inthash( temp );
			}
		};

		typedef hash_map <position, TriggerStruct, poshash, eqpos> TriggerMap;
		TriggerMap Triggers;

		void clearOldTable();

		bool _dataOK;
};

#endif
