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


#ifndef CSPAWNPOINT_HPP
#define CSPAWNPOINT_HPP

#include "globals.h"
#include "CCharacter.hpp"
#include <list>

// just declare a class named CWorld...
class CWorld;

//! defines a Spawnpoint
class CSpawnPoint {

	public:
		//! Creates a new SpawnPoint at <pos>
		CSpawnPoint(const position& pos, int Range = 20, uint16_t Spawnrange = 0, uint16_t Min_Spawntime = 1, uint16_t Max_Spawntime = 1, bool Spawnall = false );

		//! Destructor
		~CSpawnPoint();

		void addMonster(const TYPE_OF_CHARACTER_ID& typ, const short int& count);

		//! load spawnpoints from database
		bool load(const int& id);

		void spawn();

		//! callback called by dying monsters belonging to spawnpoint
		void dead(const TYPE_OF_CHARACTER_ID& typ);

		inline int get_x() const { return spawnpos.x; }
		inline int get_y() const { return spawnpos.y; }
		inline int get_z() const { return spawnpos.z; }

		inline int getRange() const { return range; }

	private:
		// our link to the world...
		CWorld* world;

		position spawnpos;
        
        //walkrange of the monsters from the spawn
		int range;
        
        //range of the spawns, in this area the creatures can be spawned
        uint16_t spawnrange;
        
        //the number of cycles untlin new monsters are spawned
        uint16_t min_spawntime;
        uint16_t max_spawntime;
        
        //the number of cycles until the next spawn
        uint16_t nextspawntime;
        
        //should be all monsters respawned in every cycle
        bool spawnall;

		struct SpawnEntryStruct {
			TYPE_OF_CHARACTER_ID typ;
			short int max_count;
			short int akt_count;
		};

		std::list<struct SpawnEntryStruct> SpawnTypes;
};

#endif // CNPC_HPP
