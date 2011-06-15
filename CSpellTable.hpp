#ifndef CSPELLTABLE_HPP
#define CSPELLTABLE_HPP

#include <map>
#include <list>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include "script/CLuaMagicScript.hpp"

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

class CWorld;

struct SpellStruct {
	//Type des aus zu führenden Zaubers.
	unsigned short magictype;
	//Name des Scriptes.
	std::string scriptname;
	//Shared Pointer auf das Script des Structs.
	boost::shared_ptr<CLuaMagicScript> script;
};

class CSpellTable {
	public:
		CSpellTable();

		~CSpellTable();

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
