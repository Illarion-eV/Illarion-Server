#ifndef CTRIGGERTABLE_HPP
#define CTRIGGERTABLE_HPP

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

#include "globals.h"
#include "script/CLuaTriggerScript.hpp"

class CWorld;

struct TriggerStruct {
	//Position des Triggerfelds
	position pos;
	//Name des Scriptes;
	std::string scriptname;
	//Zeiger auf das Script
	boost::shared_ptr<CLuaTriggerScript> script;
};



class CTriggerTable {
	public:

		CTriggerTable();
		~CTriggerTable();

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
