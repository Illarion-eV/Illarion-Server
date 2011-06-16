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


#ifndef CMONSTER_HPP
#define CMONSTER_HPP

#include "CCharacter.hpp"
#include "CSpawnPoint.hpp"
#include <CIdCounter.h>
#include <memory>

/**
* @ingroup Scriptclasses
* a monster in the game world
*/
class CMonster : public CCharacter {

	public:

        /**
        * an exception which is thrown if a id for the monster is unknown
        */
		class unknownIDException { }; 
		
        /**
        * the constructor which creates the monster on the map
        * @param type the monster type of this monster
        * @param newpos the position where the monster should be spawned
        * @param spawnpoint the spawnpoint from which the monster was spawned ( 0 if there is no spawnpoint )
        */
		CMonster(const TYPE_OF_CHARACTER_ID& type, const position& newpos, CSpawnPoint* spawnpoint=0) throw(unknownIDException);

        /**
        * sets the type of the monster and changes
        * so the attributes etc for this monster
        * @param type the new type of the monster
        * @throw unknownIDException
        */
        void setType(const TYPE_OF_CHARACTER_ID& type ) throw(unknownIDException);

		/**
        * sets the spawnpoint for thins monster to a new one
        * @param sp the pointer to the new spawnpoint
        */
		void setSpawn(CSpawnPoint* sp);
        
        /**
        * sets the alive state of the monster
        * if a monster is killed it will be removed from his spawnpoint 
        * @param t the new lifestate true = alive false = death
        */
		void SetAlive( bool t );
        
        void remove();

         
        /**
        * the monster gets a talked text from a character near starts a script entry point
        * @param tt how loudly the text is spoken
        * @param message the message which is spoken
        * @param cc the character who has spoken the message
        */
		void receiveText(talk_type tt, std::string message, CCharacter* cc);

		/**
        * returns the spawnpoint of the monster
        * @return the spawnpoint of this monster
        */
		inline CSpawnPoint* getSpawn() const { return spawn; }

        /**
        * returns the type of this monster
        * @return the type
        */
		inline TYPE_OF_CHARACTER_ID getType() { return monstertype; }
		
		/**
        * the monster is attacking another character starts a script entry
        * @param target the character which is attacked
        * @param sound which should be played ( not neccessary here inherited from ccharacter)
        * @param updateInv if the inventory should be updated ( not neccessary here inherited from ccharacter)
        * @return no usage at this time anything can be returned
        */
        bool attack( CCharacter* target, int &sound, bool &updateInv );
        
        /**
        *trys to find a path to the the targetposition and performs a step
        *in the direction
        *@param targetpos targetposition for the move;
        */
        void performStep(position targetpos);
        
		/**
        * destructor
        */
		~CMonster();
        
        position lastTargetPosition; /**< last position of the last seen target */
        bool lastTargetSeen; /**< if true the monster trys to reach the last targetposition if there is no other enemy*/
        
        /**
        * checks if the monster can attack onther one or it is a peacefull monster
        * @return true if the monster is aggresive otherwise false
        */
		inline const bool canAttack() { return _canAttack; }
	private:
        
        /**
        * pointer to the spawnpoint of this monster
        */
		CSpawnPoint* spawn;
        
        /**
        *pointer to the id counter which gives new unique id's to the monsters
        */
		static std::auto_ptr<CIdCounter> monsteridc;
		
        /**
        * stores the type of the monster
        */
        TYPE_OF_CHARACTER_ID monstertype;
        
        /**
        * stores if the monster can attack or not
        */
		bool _canAttack;


};

#endif // CMONSTER_HPP
