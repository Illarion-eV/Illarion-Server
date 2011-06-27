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


#include "Monster.hpp"
#include "Random.hpp"
#include "tuningConstants.hpp"
#include "CommonObjectTable.hpp"
#include <iostream>
#include "MonsterTable.hpp"
#include <boost/shared_ptr.hpp>
#include "script/LuaMonsterScript.hpp"
#include "World.hpp"

std::auto_ptr<IdCounter> Monster::monsteridc;

// the table with monster descriptions
extern MonsterTable* MonsterDescriptions;

Monster::Monster(const TYPE_OF_CHARACTER_ID& type, const position& newpos, SpawnPoint* spawnpoint) throw(unknownIDException)
		: Character(),lastTargetPosition(position(0,0,0)),lastTargetSeen(false), spawn(spawnpoint), monstertype(type) {

	if (monsteridc.get() == 0)
		monsteridc.reset(new IdCounter(configOptions["monsteridc"], MONSTER_BASE)); // reset monster id to 0xBB000000

	character = monster;
	actionPoints = NP_MAX_AP;
	SetAlive( true );
	setType(type);
	pos=newpos;
}

void Monster::performStep( position targetpos )
{
	Character::direction dir;
	if (getNextStepDir(targetpos, MAX_PATH_FIND, dir) )
	{
		std::cout << "performStep here!   " << dir << std::endl;
		move( dir );   
		std::cout << "performStep here! pass 1" << std::endl;
	}
	else
	{
		dir = static_cast<Character::direction>(unsignedShortRandom( 0, 7));
		move( dir );
	}
    // unsigned char direction = 0;
	// short int xoffs;
	// short int yoffs;
	// short int zoffs;
					
	// xoffs = targetpos.x - pos.x;
	// yoffs = targetpos.y - pos.y;
	// zoffs = targetpos.z - pos.z;
	// if ( ( abs(xoffs) <= MAX_PATH_FIND ) && ( abs(yoffs) <= MAX_PATH_FIND ) ) 
    // {
		// // player near... start breadth-first search to find a way towards...
		// // should be replaced by A* search later...
        // bool karte[MAX_PATH_FIND * 2 + 1][MAX_PATH_FIND * 2 + 1];
		// char distances[MAX_PATH_FIND * 2 + 1][MAX_PATH_FIND * 2 + 1];
		// SuchFeldListe Wertungen;
		// Field* temp4;
		// for ( int x = 0; x < MAX_PATH_FIND * 2 + 1; ++x ) 
        // {
			// for ( int y = 0; y < MAX_PATH_FIND * 2 + 1; ++y ) 
            // {
                // if (_world->GetPToCFieldAt(temp4,pos.x-MAX_PATH_FIND+x,pos.y-MAX_PATH_FIND+y,pos.z)) 
                // {
					// karte[x][y]=!(temp4->moveToPossible());
				// } 
                // else 
                // {
					// karte[x][y]=true;
				// }
			// }
		// }
		// karte[MAX_PATH_FIND][MAX_PATH_FIND] = false;
		// karte[MAX_PATH_FIND+xoffs][MAX_PATH_FIND+yoffs] = false; // we want to move to our target...
		// feld aktFeld, neuFeld;
		// for ( int x = 0; x < MAX_PATH_FIND * 2 + 1; ++x )
			// for ( int y = 0; y < MAX_PATH_FIND * 2 + 1; ++y )
				// distances[x][y] = 127;
		// aktFeld.x=MAX_PATH_FIND;
		// aktFeld.y=MAX_PATH_FIND;
		// aktFeld.distance=0;
		// Wertungen.push_back(aktFeld);

		// while (!Wertungen.empty()) 
        // {
			// aktFeld = Wertungen[0];
			// Wertungen.erase(Wertungen.begin());
			// if ( distances[aktFeld.x][aktFeld.y] > aktFeld.distance )
				// distances[aktFeld.x][aktFeld.y] = aktFeld.distance;
			// if ( ! karte[aktFeld.x][aktFeld.y] )//Karte frei
			// {
				// neuFeld.distance = aktFeld.distance + 1;
				// neuFeld.x = aktFeld.x;
				// if ( aktFeld.y > 0 ) 
                // {
					// neuFeld.y = aktFeld.y-1;
					// if ( ( distances[neuFeld.x][neuFeld.y] > neuFeld.distance ) && ( ! karte[neuFeld.x][neuFeld.y] ) ) 
                    // {
						// distances[neuFeld.x][neuFeld.y] = neuFeld.distance;
						// Wertungen.push_back(neuFeld);
					// }
				// }
				// if ( aktFeld.y < MAX_PATH_FIND * 2 ) 
                // {
					// neuFeld.y = aktFeld.y + 1;
					// if ( ( distances[neuFeld.x][neuFeld.y] > neuFeld.distance ) && ( ! karte[neuFeld.x][neuFeld.y] ) ) 
                    // {
						// distances[neuFeld.x][neuFeld.y] = neuFeld.distance;
						// Wertungen.push_back(neuFeld);
					// }
				// }
				// neuFeld.y = aktFeld.y;
				// if ( aktFeld.x > 0 ) 
                // {
					// neuFeld.x = aktFeld.x - 1;
                    // if ( ( distances[neuFeld.x][neuFeld.y] > neuFeld.distance ) && ( ! karte[neuFeld.x][neuFeld.y] ) ) 
                    // {
                        // distances[neuFeld.x][neuFeld.y] = neuFeld.distance;
						// Wertungen.push_back(neuFeld);
					// }
				// }
				// if ( aktFeld.x < MAX_PATH_FIND * 2 ) 
                // {
					// neuFeld.x = aktFeld.x + 1;
					// if ( ( distances[neuFeld.x][neuFeld.y] > neuFeld.distance ) && ( ! karte[neuFeld.x][neuFeld.y] ) ) 
                    // {
						// distances[neuFeld.x][neuFeld.y] = neuFeld.distance;
						// Wertungen.push_back(neuFeld);
					// }
				// }
			// }
		// }
		// feld path;
		// // starting from our target...
		// path.x = MAX_PATH_FIND + xoffs;
		// path.y = MAX_PATH_FIND + yoffs;
		// path.distance = distances[path.x][path.y];
		// while (path.distance > 1 && path.distance < 127) 
        // {
			// // find out which field to go to...
			// if (path.x > 0 && distances[path.x-1][path.y] < path.distance) 
            // {
				// path.x--;
				// path.distance = distances[path.x][path.y];
				// continue;
			// } 
            // else if (path.x <2*MAX_PATH_FIND  && distances[path.x+1][path.y] < path.distance) 
            // {
				// path.x++;
				// path.distance = distances[path.x][path.y];
				// continue;
			// } 
            // else if (path.y >0 && distances[path.x][path.y-1] < path.distance) 
            // {
				// path.y--;
				// path.distance = distances[path.x][path.y];
				// continue;
			// } 
            // else if (path.y <2*MAX_PATH_FIND  && distances[path.x][path.y+1] < path.distance) 
            // {
				// path.y++;
				// path.distance = distances[path.x][path.y];
				// continue;
			// }
		// }
		// if (path.x > MAX_PATH_FIND) // east
			// direction = 2;
		// if (path.x < MAX_PATH_FIND) // west
			// direction = 6;
		// if (path.y > MAX_PATH_FIND) // south
			// direction = 4;
		// if (path.y < MAX_PATH_FIND) // north
			// direction = 0;

		// // if we can't reach our target... just make a random move... some movement is better then none at all :)
		// if (path.distance == 127)
			// direction = 2 * unsignedShortRandom( 0, 3);

	// }
    // else //Nur in die richtung laufen
	// {
		// if (abs(xoffs) > abs(yoffs) ) 
        // {
			// direction = (xoffs>0)?6:2;
		// } 
        // else 
        // {
			// direction = (yoffs>0)?4:0;
		// }
	// }
	//move( (Character::direction)direction );    
}

void Monster::setType( const TYPE_OF_CHARACTER_ID& type ) throw(unknownIDException) {
	deleteAllSkills();


	MonsterStruct monsterdef;

	if (! MonsterDescriptions->find(type, monsterdef) )
		throw unknownIDException();

	// set attributes
	battrib.luck = battrib.trueluck = rnd( monsterdef.attributes.luck );
	battrib.strength = battrib.truestrength = rnd( monsterdef.attributes.strength );
	battrib.dexterity = battrib.truedexterity = rnd( monsterdef.attributes.dexterity );
	battrib.constitution = battrib.trueconstitution = rnd( monsterdef.attributes.constitution );
	battrib.agility = battrib.trueagility = rnd( monsterdef.attributes.agility );
	battrib.intelligence = battrib.trueintelligence = rnd( monsterdef.attributes.intelligence );
	battrib.perception = battrib.trueperception = rnd( monsterdef.attributes.perception );
	battrib.willpower = battrib.truewillpower = rnd( monsterdef.attributes.willpower );
	battrib.essence = battrib.trueessence = rnd( monsterdef.attributes.essence );
	battrib.hitpoints = battrib.truehitpoints = monsterdef.hitpoints; //Setzen der Hitpoints
	battrib.body_height = battrib.truebody_height = rand()%(monsterdef.maxsize-monsterdef.minsize+1) + monsterdef.minsize;

	// set skills
	for (MonsterStruct::skilltype::iterator it = monsterdef.skills.begin(); it != monsterdef.skills.end(); ++it)
		increaseSkill( 5, it->first, rnd( it->second ) );

	// add items
	int itempropability;
	bool found;
	for (MonsterStruct::itemtype::iterator it = monsterdef.items.begin(); it != monsterdef.items.end(); ++it) {
		// check if we should equip the item...
		found = false;
		itempropability = rnd(1,1000);
		for (std::list<itemdef_t>::iterator itemit = it->second.begin(); !found && itemit != it->second.end(); ++itemit) {
			//std::cout<<"Itemprop for item: "<< itemit->itemid<<" prob: "<<itempropability<<" Itemit prob="<<itemit->propability<<"\n";
			if (itemit->propability <= itempropability) {
				//std::cout<<"Added Item:"<<itemit->itemid<<"\n";
				characterItems[ it->first ].id = itemit->itemid;     // Eingeweide
				characterItems[ it->first ].number = rnd(itemit->amount);
				characterItems[ it->first ].wear = itemit->AgeingSpeed;
				found = true;
			}
			itempropability += itemit->propability;
		}
	}

	SetMovement(monsterdef.movement);
	race = monsterdef.race;
	_canAttack = monsterdef.canattack;
	id=monsteridc->nextFreeId();
	char mname[ 80 ];
	sprintf ( mname, "%0#6X %s", id, monsterdef.name.c_str() );
	name=mname;

	appearance = appearance_alive();
}

void Monster::setSpawn(SpawnPoint* sp) {
	spawn = sp;
}

Monster::~Monster() {
#ifdef Character_DEBUG
	std::cout << "Monster Destruktor Start" << std::endl;
#endif
	if (spawn) spawn->dead(monstertype);
#ifdef Character_DEBUG
	std::cout << "Monster Destruktor Ende" << std::endl;
#endif
}

void Monster::remove()
{
    battrib.truehitpoints = 0;
    battrib.hitpoints = battrib.truehitpoints;
    Character::SetAlive(false);
    
}

void Monster::SetAlive( bool t) {
	if ( !t ) {
		
		MonsterStruct monStruct;
		if ( MonsterDescriptions->find(getType(), monStruct) ) {
			if ( monStruct.script ) {
				monStruct.script->onDeath(this);
			} else {
				std::cerr<<"Script for Monster: "<<getType()<<" not active!"<<std::endl;
			}
		} else {
			std::cerr<<"Can't finde Description for Monster: " << getType() << " on Death not called!"<<std::endl;
		}
	}
	Character::SetAlive(t);
}

bool Monster::attack(Character* target, int &sound, bool &updateInv)
{

    MonsterStruct monStruct;
    if ( MonsterDescriptions->find(getType(), monStruct) )
    {
        if ( monStruct.script )
            monStruct.script->onAttack(this,target);
        else
            std::cerr<<"Script for Monster: "<<getType()<<"not active!"<<std::endl;
    }
    else
        std::cerr<<"Can't find Description for Monster: " << getType() << " onAttack not called!" << std::endl;
    return Character::attack(target,sound,updateInv);
}

void Monster::receiveText(talk_type tt, std::string message, Character* cc) {
	MonsterStruct monStruct;
	if ( MonsterDescriptions->find(getType(), monStruct) ) {
		if ( monStruct.script!=NULL && monStruct.script->existsEntrypoint("receiveText") )
        {
			//Nur Script aufrufen wenn man sich nicht selber hört.
			if ( this != cc ) monStruct.script->receiveText(this,tt,message,cc);
		}
	} else {
		std::cerr<<"Can't find description for monster: " << getType() << " receiveText not called!"<<std::endl;
	}
}
