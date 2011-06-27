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


#ifndef SPELLTABLE_HPP
#define SPELLTABLE_HPP

#include <map>
#include <list>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "script/LuaMagicScript.hpp"

#if __GNUC__ < 3
#include <hash_map>
#else
#include <ext/hash_map>

#if (__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
using __gnu_cxx::hash_multimap;
#endif

#if (__GNUC__ == 3 && __GNUC_MINOR__ < 1)
using std::hash_multimap;
#endif

#endif

class World;

struct SpellStruct {
	//Type des aus zu führenden Zaubers.
	unsigned short magictype;
	//Name des Scriptes.
	std::string scriptname;
	//Shared Pointer auf das Script des Structs.
	boost::shared_ptr<LuaMagicScript> script;
};

class SpellTable {
	public:
		SpellTable();

		~SpellTable();

		void reload();

		inline bool isDataOK() { return _dataOK; }

		bool find( unsigned long int magicfFlag,unsigned short int magic_type, SpellStruct &magicSpell);

	protected:

		typedef hash_multimap<unsigned long int,SpellStruct> SpellMap;
		SpellMap Spells;

		void clearOldTable();

		bool _dataOK;

};
#endif
