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


#include "db/ConnectionManager.hpp"
#include "World.hpp"
#include "WeaponObjectTable.hpp"
#include "TilesModificatorTable.hpp"
#include "ContainerObjectTable.hpp"
#include "CommonObjectTable.hpp"
#include "ArmorObjectTable.hpp"
#include "MonsterTable.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include <list>
//#include <algorith>
#include <stdlib.h>

//Table with data of Monsters
extern CMonsterTable* MonsterDescriptions;

class CField;

//function is quick and dirty, should be replaced later
void CWorld::deleteAllLostNPC()
{
	CField* tempf; //alte NPC's l?chen
	NPCVECTOR::iterator npcIteratorOld;
    std::vector< TYPE_OF_CHARACTER_ID>::iterator npcIteratorToDelete;
    for ( npcIteratorToDelete = LostNpcs.begin(); npcIteratorToDelete != LostNpcs.end(); ++npcIteratorToDelete)
    {
        std::cout<<" Delete NPC: "<<(*npcIteratorToDelete)<<std::endl;
        for ( npcIteratorOld = Npc.begin(); npcIteratorOld < Npc.end(); ++npcIteratorOld ) 
        {
            if (( *npcIteratorOld )->id == (*npcIteratorToDelete) ) {
                if ( GetPToCFieldAt( tempf, ( *npcIteratorOld )->pos.x, ( *npcIteratorOld )->pos.y, ( *npcIteratorOld )->pos.z ) ) 
                {
                    //tempf->SetNPCOnField( false );
                    tempf->removeChar();
                }
                
                sendRemoveCharToVisiblePlayers( ( *npcIteratorOld )->id, ( *npcIteratorOld )->pos ); 
                delete( *npcIteratorOld );
                npcIteratorOld = Npc.erase( npcIteratorOld );
            }
        }
        std::cout<<" NPC was deleted"<<std::endl;
    }
    LostNpcs.clear();
}

bool CWorld::findPlayersInSight(position pos, uint8_t range, std::vector<CPlayer*> &ret, CCharacter::face_to direction)
{
    bool found = false;
    std::vector<CPlayer*>playersinrange = Players.findAllAliveCharactersInRangeOfOnSameMap(pos.x,pos.y,pos.z,range);
    for ( std::vector<CPlayer*>::iterator pIterator = playersinrange.begin(); pIterator != playersinrange.end(); ++pIterator)
    {
        
        bool indir = false;
        switch ( direction )
        {
            case CCharacter::north:
                if ( (*pIterator)->pos.y <= pos.y ) indir = true;
                break;
            case CCharacter::northeast:
                if ( (*pIterator)->pos.x - pos.x >= (*pIterator)->pos.y - pos.y ) indir = true;
                break;
            case CCharacter::east:
                if ( (*pIterator)->pos.x >= pos.x ) indir = true;
                break;
            case CCharacter::southeast:
                if ( (*pIterator)->pos.y - pos.y >= pos.x - (*pIterator)->pos.x ) indir = true;
                break;
            case CCharacter::south:
                if ( (*pIterator)->pos.y >= pos.y ) indir = true;
                break;
            case CCharacter::southwest:
                if ( (*pIterator)->pos.x - pos.x <= (*pIterator)->pos.y - pos.y ) indir = true;
                break;
            case CCharacter::west:
                if ( (*pIterator)->pos.x <= pos.x ) indir = true;
                break;
            case CCharacter::northwest:
                if ( (*pIterator)->pos.y - pos.y >= pos.x - (*pIterator)->pos.x ) indir = true;
                break;
            default:
                indir = true;
                break;
        }
        if (indir)
        {
            std::list<BlockingObject> objects = LoS(pos,(*pIterator)->pos);
            if ( objects.empty() )
            {
                ret.push_back( (*pIterator) );
                found = true;
            }
        }
    }
    return found;
}

std::list<BlockingObject> CWorld::LoS(position startingpos, position endingpos)
{
    std::list<BlockingObject> ret;
    ret.clear();
    bool steep = std::abs(startingpos.y - endingpos.y) > std::abs(startingpos.x - endingpos.x);
    short int startx=startingpos.x;
    short int starty=startingpos.y;
    short int endx=endingpos.x;
    short int endy=endingpos.y;
    if ( steep )
    {
        //change x,y values for correct algorithm in negativ range
        short int change;
        change = startx;
        startx = starty;
        starty = change;
        change = endx;
        endx = endy;
        endy = change;
    }
    bool swapped = startx > endx; 
    if ( swapped )
    {
        short int change;
        change = startx;
        startx = endx;
        endx = change;
        change = starty;
        starty = endy;
        endy = change;
        
    }
    short int deltax = endx - startx;
    short int deltay = std::abs(endy - starty);
    short int error = 0;
    short int ystep=1;
    short int y = starty;
    if ( starty > endy ) ystep = -1;
    unsigned short int effekt = 1;
    for ( short int x = startx; x <= endx; ++x)
    {
        if ( !(x == startx && y == starty ) && !(x == endx && y == endy) )
        {            
            BlockingObject bo;
            CField * temp;
            if (steep)
            {
                //makeGFXForAllPlayersInRange(y,x,startingpos.z, MAXVIEW,0);
                if ( GetPToCFieldAt(temp,y,x,startingpos.z) )
                {
                    if ( temp->IsPlayerOnField() )
                    {
                        //makeGFXForAllPlayersInRange(y,x,startingpos.z, MAXVIEW,effekt);
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(y,x,startingpos.z);
                        if (swapped) ret.push_back(bo);
                        else ret.push_front(bo);
                    }
                    else if ( !temp->IsPassable() )
                    {
                        //makeGFXForAllPlayersInRange(y,x,startingpos.z, MAXVIEW,effekt);
                        ScriptItem it;
                        if ( temp->ViewTopItem( it ) )
                        {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = position(y,x,startingpos.z);
                            it.type = ScriptItem::it_field;
                            bo.blockingItem = it;
                            if (swapped) ret.push_back(bo);
                            else ret.push_front(bo);
                        }
                    }
                    //else
                    //    makeGFXForAllPlayersInRange(y,x,startingpos.z, MAXVIEW,0);
                    
                }
            }
            else
            {
                //makeGFXForAllPlayersInRange(x,y,startingpos.z, MAXVIEW,0);
                if ( GetPToCFieldAt(temp,x,y,startingpos.z) )
                {
                    if ( temp->IsPlayerOnField() )
                    {
                        //makeGFXForAllPlayersInRange(x,y,startingpos.z, MAXVIEW,effekt);
                        bo.blockingType = BlockingObject::BT_CHARACTER;
                        bo.blockingChar = findCharacterOnField(x,y,startingpos.z);
                        if (swapped) ret.push_back(bo);
                        else ret.push_front(bo);
                    }
                    else if ( !temp->IsPassable() )
                    {
                        //makeGFXForAllPlayersInRange(x,y,startingpos.z, MAXVIEW,effekt);
                        ScriptItem it;
                        if ( temp->ViewTopItem( it ) )
                        {
                            bo.blockingType = BlockingObject::BT_ITEM;
                            it.pos = position(x,y,startingpos.z);
                            it.type = ScriptItem::it_field;
                            bo.blockingItem = it;
                            if (swapped) ret.push_back(bo);
                            else ret.push_front(bo);
                        }
                    }
                    //else
                    //    makeGFXForAllPlayersInRange(x,y,startingpos.z, MAXVIEW,0);
                    
                }
            }
        }
        error += deltay;
        if ( 2*error >= deltax )
        {
            y+=ystep;
            error -= deltax;
        }
        effekt = 2;
       
    }
    return ret;
}

//function which updates the playerlist.
void CWorld::updatePlayerList()
{
    std::cout<<"Updateplayerlist start"<<std::endl;
    ConnectionManager::TransactionHolder transaction = dbmgr->getTransaction();
    try 
    {
        //empty the list
        di::exec(transaction, "DELETE FROM onlineplayer");
        PLAYERVECTOR::iterator plIterator;
        for ( plIterator = Players.begin(); plIterator != Players.end(); ++plIterator)
        {
            di::insert(transaction, (*plIterator)->id, "INSERT INTO onlineplayer (on_playerid)");
        }
        transaction.commit();
    }
    catch (std::exception e)
    {
        std::cerr<<"caught exception during online player save: "<<e.what()<<std::endl;
        transaction.rollback();
    }
    std::cout<<"updateplayerlist end"<<std::endl;
}

CCharacter* CWorld::findCharacterOnField(short int posx, short int posy, short int posz) 
{
	CCharacter* tmpChr;
	tmpChr = Players.find(posx, posy, posz);
	if ( tmpChr != NULL )
		return tmpChr;
	tmpChr = Monsters.find(posx, posy, posz);
	if ( tmpChr != NULL )
		return tmpChr;
	tmpChr = Npc.find(posx, posy, posz);
	if ( tmpChr != NULL )
		return tmpChr;
	return NULL;
}

CPlayer* CWorld::findPlayerOnField(short int posx, short int posy, short int posz) 
{
	return Players.find(posx, posy, posz);
}

CCharacter* CWorld::findCharacter(TYPE_OF_CHARACTER_ID id) {
	CCharacter* tmpChr;
	tmpChr = dynamic_cast<CCharacter*>(Players.findID(id));
	if ( tmpChr != NULL )
		return tmpChr;
	tmpChr = dynamic_cast<CCharacter*>(Monsters.findID(id));
	if ( tmpChr != NULL )
		return tmpChr;
	tmpChr = dynamic_cast<CCharacter*>(Npc.findID(id));
	if ( tmpChr != NULL )
		return tmpChr;
	return NULL;
}


bool CWorld::findPlayerWithLowestHP( std::vector < CPlayer* > * ppvec, CPlayer* &found ) {
	found = NULL;
	std::vector < CPlayer* > ::iterator pIterator;
	for ( pIterator = ppvec->begin(); pIterator < ppvec->end(); ++pIterator ) {
		if ( found == NULL ) {
			found = *pIterator;
		} else {
			if ( found->battrib.hitpoints > ( *pIterator )->battrib.hitpoints ) {
				found = ( *pIterator );
			}
		}
	}

	return ( found != NULL );
}


void CWorld::takeMonsterAndNPCFromMap() {
	CField* tempf;

	MONSTERVECTOR::iterator monsterIterator;
	for ( monsterIterator = Monsters.begin(); monsterIterator < Monsters.end(); ++monsterIterator ) {
		if ( GetPToCFieldAt( tempf, ( *monsterIterator )->pos.x, ( *monsterIterator )->pos.y, ( *monsterIterator )->pos.z ) ) {
			tempf->SetMonsterOnField( false );
		}
		delete( *monsterIterator );
		*monsterIterator = NULL;
	}

	NPCVECTOR::iterator npcIterator;
	for ( npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator ) {
		if ( GetPToCFieldAt( tempf, ( *npcIterator )->pos.x, ( *npcIterator )->pos.y, ( *npcIterator )->pos.z ) ) {
			tempf->SetNPCOnField( false );
		}
		delete( *npcIterator );
		*npcIterator = NULL;
	}

	Monsters.clear();
	Npc.clear();
}


// only invoked when ATTACK***_TS is received or when a monster attacks
bool CWorld::characterAttacks( CCharacter* cp ) {

	if (cp->enemyid != cp->id) 
    {
		int sound = 0;
		bool updateInv = false;
		if ( cp->enemytype == CCharacter::player )
        {
#ifdef CWorld_DEBUG
			std::cout << "attack player" << std::endl;
#endif
			CPlayer* temppl = Players.findID( cp->enemyid );

			// Ziel gefunden
			if ( temppl != NULL ) {
				// Ziel sichtbar
				if ( cp->isInRange( temppl, MAXVIEW ) ) {
					int temphp = temppl->battrib.hitpoints;
					// Ziel ist tot
					if ( !cp->attack( temppl, sound, updateInv ) ) 
                    {
						sendSpinToAllVisiblePlayers( temppl );
// There are no yellow crosses any more, however, one might want a different message in the future.
// Currently no message is desired.
/* 						
                        if (temppl->getPlayerLanguage() == CLanguage::german) 
                        {
                            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( cp->pos.x, cp->pos.y, cp->pos.z, "Geh zum gelben Kreuz, um wiederbelebt zu werden!") );
                            temppl->Connection->addCommand(cmd);
						} 
                        else 
                        {
                            boost::shared_ptr<CBasicServerCommand>cmd( new CSayTC( cp->pos.x, cp->pos.y, cp->pos.z, "Walk to the YELLOW CROSS to be resurrected!") );
                            temppl->Connection->addCommand(cmd);
						}
*/

						cp->attackmode = false;
                        
                        //set lasttargetseen to false if the player who was attacked is death
                        if ( cp->character == CCharacter::monster )
                        {
                            CMonster * mon = dynamic_cast<CMonster*>(cp);
                            mon->lastTargetSeen = false;
                        }

						// dead people cannot be attacked, reset counter
						//temppl->nrOfAttackers=0;

						if ( cp->character == CCharacter::player )
                        {
                            boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC() );
                            dynamic_cast<CPlayer*>(cp)->Connection->addCommand(cmd);
						}
                        boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC() );
						dynamic_cast<CPlayer*>(temppl)->Connection->addCommand(cmd);
						temppl->attackmode = false;
					}
#ifdef DO_UNCONSCIOUS
					else if ( ! temppl->IsConscious() ) {
						sendSpinToAllVisiblePlayers( temppl, PLAYERSPIN_TC );
					}
#endif
					// player should know that he is target of an attack (of another player or NPC)
					//temppl->nrOfAttackers++;

					if ( ( cp->character == CCharacter::player ) && (updateInv) ) {
						( ( CPlayer* ) cp )->sendCharacterItemAtPos( LEFT_TOOL );
						( ( CPlayer* ) cp )->sendCharacterItemAtPos( RIGHT_TOOL );
					}

					// bewirkt nur ein Update beim Client
					if (temphp != temppl->battrib.hitpoints) {
						temppl->sendAttrib("hitpoints",temppl->increaseAttrib( "hitpoints", 0 ) );
						makeGFXForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, 13 );
					}
					if (sound != 0) {
						makeSoundForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, sound );
					}

					return true;

				}
			}
		} else if ( cp->enemytype == CCharacter::monster ) {
#ifdef CWorld_DEBUG
			std::cout << "attack monster" << std::endl;
#endif

			CMonster* temppl = Monsters.findID( cp->enemyid );

			// Ziel gefunden
			if ( temppl != NULL ) {
				if ( cp->isInRange( temppl, MAXVIEW) ) 
                {
					int temphp = temppl->battrib.hitpoints;
					MonsterStruct monStruct;
					if ( MonsterDescriptions->find( temppl->getType(), monStruct) ) 
                    {
						if (monStruct.script) 
                        {
							monStruct.script->onAttacked(temppl,cp);
						} 
                        else 
                        {
							std::cerr<<"No script initialized for monster: "<<temppl->getType()<<" on Attack not called!"<<std::endl;
						}
					} 
                    else 
                    {
						std::cerr<<"Didn't finde Monster Description for: "<< temppl->getType() << " can't call onAttacked!"<<std::endl;
					}
					// Ziel ist tot
					if ( !cp->attack( temppl, sound, updateInv ) ) 
                    {
						cp->attackmode = false;
						if ( cp->character == CCharacter::player ) 
                        {
                            boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC());
							dynamic_cast<CPlayer*>(cp)->Connection->addCommand(cmd);
						}
					}
                    else
                    {
                        //check for turning into attackackers direction
                        std::vector<CPlayer*>temp;
                        temp.clear();
                        findPlayersInSight( temppl->pos, static_cast<uint8_t>(9), temp, temppl->faceto);
                        //add the current attacker to the list
                        if ( cp->character == CCharacter::player ) temp.push_back( dynamic_cast<CPlayer*>(cp) );
                        CPlayer * foundPl;
                        if ( !temp.empty() && findPlayerWithLowestHP( &temp, foundPl ) )
                        {
                            temppl->turn( foundPl->pos );
                        }
                        
                    }

					if ( ( cp->character == CCharacter::player ) && (updateInv) ) {
						( ( CPlayer* ) cp )->sendCharacterItemAtPos( LEFT_TOOL );
						( ( CPlayer* ) cp )->sendCharacterItemAtPos( RIGHT_TOOL );
					}

					if (temphp != temppl->battrib.hitpoints) {
						makeGFXForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, 13 );
					}
					if (sound != 0) {
						makeSoundForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, sound );
					}
					return true;
				}
			}
		} else if ( cp->enemytype == CCharacter::npc && false ) // Disable NPC attacks
		{
#ifdef CWorld_DEBUG
			std::cout << "attack npc" << std::endl;
#endif

			CNPC* temppl = Npc.findID( cp->enemyid );

			// Ziel gefunden
			if ( temppl != NULL ) {
				if ( cp->isInRange( temppl, MAXVIEW ) ) {
					int temphp=temppl->battrib.hitpoints;
					// Ziel ist tot
					if ( !cp->attack( temppl, sound, updateInv ) ) {
						cp->attackmode = false;
						if ( cp->character == CCharacter::player ) 
                        {
                            boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC() );
							dynamic_cast<CPlayer*>(cp)->Connection->addCommand(cmd);
						}
					}

					if ( ( cp->character == CCharacter::player ) && (updateInv) ) {
						dynamic_cast<CPlayer*>(cp)->sendCharacterItemAtPos( LEFT_TOOL );
						dynamic_cast<CPlayer*>(cp)->sendCharacterItemAtPos( RIGHT_TOOL );
					}

					if (temphp != temppl->battrib.hitpoints) {
						makeGFXForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, 13 );
					}
					if (sound != 0) {
						makeSoundForAllPlayersInRange( temppl->pos.x, temppl->pos.y, temppl->pos.z, MAXVIEW, sound );
					}
					return true;
				}
			}
		}

		// Ziel nicht gefunden/außer Sichtweite
		cp->attackmode = false;
		if ( cp->character == CCharacter::player ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd( new CTargetLostTC() );
			dynamic_cast<CPlayer*>(cp)->Connection->addCommand(cmd);
		}

		return false;
	} else {
		return true;
	}

}


bool CWorld::killMonster( CMonster* monsterp ) {
	if ( monsterp != NULL ) {
		MONSTERVECTOR::iterator newIt;
		if ( Monsters.getIterator( monsterp->id, newIt ) ) {
			MONSTERVECTOR::iterator temp;
			killMonster( newIt, temp );
			return true;
		}
	}

	return false;
}


void CWorld::killMonster( MONSTERVECTOR::iterator monsterIt, MONSTERVECTOR::iterator &newIt ) {

	//( *monsterIt )->SetAlive( false );
	CField* tempf;
	if ( GetPToCFieldAt( tempf, ( *monsterIt )->pos.x, ( *monsterIt )->pos.y, ( *monsterIt )->pos.z ) ) {
		//tempf->SetMonsterOnField( false );
		tempf->removeChar();
	} else {
#ifdef CWorld_DEBUG
		std::cout << "Feld nicht gefunden" << std::endl;
#endif

	}

    sendRemoveCharToVisiblePlayers( (*monsterIt)->id, (*monsterIt)->pos );

	/*for ( int i = MAX_BELT_SLOTS + MAX_BODY_ITEMS - 1; i >= 0; --i ) {
		dropItemFromMonsterOnMap( ( *monsterIt ), i, 0, 0, 0, MAXITEMS );
	}*/

	// update our monster-list counts (temporary solution TODO)
	char num;
	switch ((*monsterIt)->race) {
		case CCharacter::mumie:
			num = 0;
			break;
		case CCharacter::beholder:
			num = 1;
			break;
		case CCharacter::insects:
			num = 2;
			break;
		case CCharacter::sheep:
			num = 3;
			break;
		case CCharacter::spider:
			num = 4;
			break;
		case CCharacter::demonskeleton:
			num = 5;
			break;
		case CCharacter::rotworm:
			num = 6;
			break;
		case CCharacter::bigdemon:
			num = 7;
			break;
		case CCharacter::skeleton:
			num = 8;
			break;
		case CCharacter::pig:
			num = 9;
			break;
		case CCharacter::scorpion:
			num = 10;
			break;
		case CCharacter::troll:
			num = 11;
			break;
		default:
			num = -1;
	}

	// delete our monster
	if ( *monsterIt ) {
		delete *monsterIt;
	}
	newIt = Monsters.erase( monsterIt );

}


bool CWorld::doHealing( CCharacter* cc ) {

#ifdef DO_UNCONSCIOUS
	if ( cc->IsDying() ) {
		cc->increaseAttrib( "hitpoints", -UNCONSCIOUSHPGAIN );
		return true;
	}

	if ( ! cc->IsConscious() ) {
		cc->increaseAttrib( "hitpoints", UNCONSCIOUSHPGAIN );
		return true;
	}
#endif

	if ( cc->character == CCharacter::player ) {
		/**
        if ( ( cc->battrib.truefoodlevel >= 300 ) && ( ( cc->battrib.truehitpoints < MAXHPS ) || ( cc->battrib.truemana < MAXMANA ) ) ) {
			( ( CPlayer* ) cc )->increaseAttrib( "hitpoints", 100 );
			( ( CPlayer* ) cc )->increaseAttrib( "mana", 100 );
			( ( CPlayer* ) cc )->increaseAttrib( "foodlevel", -300 );
			return true;
		} else {
			return false;
		}
        */
        return false;
	} else {
		if ( ( cc->battrib.hitpoints < MAXHPS ) || ( cc->battrib.mana < MAXMANA ) ) {
			cc->increaseAttrib( "hitpoints", 150 );
			cc->increaseAttrib( "mana", 150 );
			return true;
		} else {
			return false;
		}
	}

}

CField* CWorld::GetField(position pos) {
	CField * field=NULL;
	CMap * temp=NULL;
	if ( maps.findMapForPos( pos, temp ) ) {
		if ( temp->GetPToCFieldAt( field, pos.x, pos.y) ) {
			return field;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}


bool CWorld::GetPToCFieldAt( CField* &fip, short int x, short int y, short int z ) {

	CMap * temp;
	if ( maps.findMapForPos( x, y, z, temp ) ) {
		return temp->GetPToCFieldAt( fip, x, y );
	} else {
		return false;
	}

}


bool CWorld::GetPToCFieldAt( CField* &fip, position pos ) {

	CMap * temp;
	if ( maps.findMapForPos( pos, temp ) ) {
		return temp->GetPToCFieldAt( fip, pos.x, pos.y );
	}

	return false;

}


bool CWorld::GetPToCFieldAt( CField* &fip, short int x, short int y, short int z, CMap* &map ) {

	if ( maps.findMapForPos( x, y, z, map ) ) {
		return map->GetPToCFieldAt( fip, x, y );
	}

	return false;

}


bool CWorld::GetPToCFieldAt( CField* &fip, position pos, CMap* &map ) {

	if ( maps.findMapForPos( pos, map ) ) {
		return map->GetPToCFieldAt( fip, pos.x, pos.y );
	}

	return false;

}


bool CWorld::findEmptyCFieldNear( CField* &cf, short int &x, short int &y, short int z ) 
{

	CMap * temp;
	if ( maps.findMapForPos( x, y, z, temp ) ) {
		return temp->findEmptyCFieldNear( cf, x, y );
	}

	return false;

}


int CWorld::getItemAttrib( std::string s, TYPE_OF_ITEM_ID ItemID ) 
{

	// Armor //
	if ( s == "bodyparts" ) {
		if ( ArmorItems->find( ItemID, tempArmor ) )
			return tempArmor.BodyParts;
	} else if ( s == "strokearmor" ) {
		if ( ArmorItems->find( ItemID, tempArmor ) )
			return tempArmor.StrokeArmor;
	} else if ( s == "thrustarmor" ) {
		if ( ArmorItems->find( ItemID, tempArmor ) )
			return tempArmor.ThrustArmor;
	} else if ( s == "armormagicdisturbance" ) {
		if ( ArmorItems->find( ItemID, tempArmor ) )
			return tempArmor.MagicDisturbance;
	}

	// Common //
	else if ( s == "agingspeed" ) {
		if ( CommonItems->find( ItemID, tempCommon ) )
			return tempCommon.AgeingSpeed;
	} else if ( s == "objectafterrot" ) {
		if ( CommonItems->find( ItemID, tempCommon ) )
			return tempCommon.ObjectAfterRot;
	} else if ( s == "volume" ) {
		if ( CommonItems->find( ItemID, tempCommon ) )
			return tempCommon.Volume;
	} else if ( s == "weight" ) {
		if ( CommonItems->find( ItemID, tempCommon ) )
			return tempCommon.Weight;
	}

	// Tiles Modificator //
	else if ( s == "modificator" ) {
		if ( TilesModItems->find( ItemID, tempModificator ) )
			return tempModificator.Modificator;
	}

	// Weapon //
	else if ( s == "accuracy" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.Accuracy;
	} else if ( s == "attack" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.Attack;
	} else if ( s == "defence" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.Defence;
	} else if ( s == "range" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.Range;
	} else if ( s == "weapontype" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.WeaponType;
	} else if ( s == "weaponmagicdisturbance" ) {
		if ( WeaponItems->find( ItemID, tempWeapon ) )
			return tempWeapon.MagicDisturbance;
	}

	// Container //
	else if ( s == "containervolume" ) {
		if ( ContainerItems->find( ItemID, tempContainer ) )
			return tempContainer.ContainerVolume;
	}

	return 0;

}


void CWorld::closeShowcasesForContainerPositions() 
{

	std::vector < CPlayer* > temp;
	for ( std::vector < position > ::iterator posit = contpos->begin(); posit < contpos->end(); ++posit ) {
		temp=Players.findAllCharactersInMaxRangeOf(posit->x, posit->y, posit->z, 1);
		for ( std::vector < CPlayer* > ::iterator titerator = temp.begin(); titerator < temp.end(); ++titerator ) {
			( *titerator )->closeAllShowcasesOfMapContainers();
		}
		temp.clear();
	}

}


void CWorld::updatePlayerView( short int startx, short int endx ) 
{

	std::vector < CPlayer* > temp;
	if ( Players.findAllCharactersWithXInRangeOf( startx - MAXVIEW, endx + MAXVIEW, temp ) ) 
    {
		for ( std::vector < CPlayer* > ::iterator titerator = temp.begin(); titerator < temp.end(); ++titerator ) 
        {
#ifdef CWorld_DEBUG
			std::cout << "update view for player " << ( *titerator )->name << " " << startx << ":#" << ( *titerator )->pos.x << "#:" << endx << "\n";
#endif

            (*titerator)->sendFullMap();
			sendAllVisibleCharactersToPlayer( ( *titerator ), true );
		}
	}

}


void CWorld::do_LongTimeEffects( CCharacter* cc ) {

	if ( cc != NULL ) 
    {
		if ( cc->IsAlive() ) 
        {
			if (cc->character==CCharacter::player) 
            {
				doHealing( cc );
				//( ( CPlayer* ) cc )->sendAttrib("hitpoints", cc->increaseAttrib( "hitpoints", 0 ) );
			}

			CField * fip;
			if ( GetPToCFieldAt( fip, cc->pos ) ) {
				checkFieldAfterMove( cc, fip );
			}
		}
	}

}


bool CWorld::DoAge() {

	if ( nextXtoage >= maps.getHighX() ) {
		// auf allen Karten alles abgearbeitet
		time_t temp = time( NULL );    // liefert die Sekunden seit dem 1.1.1970
		realgap = temp - last_age;
		// Zeit für neuen Durchlauf der Karte
		if ( realgap >= gap ) {
#ifdef CWorld_DEBUG
			std::cout << "CWorld.DoAge: Karte gealtert nach " << realgap << " Sekunden\n";
#endif
			++timecount;
			// Bestimmen welche Steps gealtert werden müssen
			if ( timecount == STEP_7 ) {
				AgeItem = AgeItemUpStep7;
				timecount = 0;
			} else if ( timecount % STEP_6 == 0 ) {
				AgeItem = AgeItemUpStep6;
			} else if ( timecount % STEP_5 == 0 ) {
				AgeItem = AgeItemUpStep5;
			} else if ( timecount % STEP_4 == 0 ) {
				AgeItem = AgeItemUpStep4;
			} else if ( timecount % STEP_3 == 0 ) {
				AgeItem = AgeItemUpStep3;
			} else if ( timecount % STEP_2 == 0 ) {
				AgeItem = AgeItemUpStep2;
			} else {
				AgeItem = AgeItemUpStep1;
			}

			last_age = temp;
			nextXtoage = maps.getLowX();

			//  Item im Inventory sollen nicht mehr altern, darum ist AgeInv.. auskommentiert
			AgeCharacters();
            AgeInventory(AgeItem);

			CMap::CONTAINERHASH::iterator conmap;
			CContainer::CONTAINERMAP::iterator cmi;
			// für alle Karten die Containerinhalte altern
			for ( CMapVector::iterator mapI = maps.begin(); mapI < maps.end(); ++mapI ) {
				// alle ContainerMap auf dem Feld
				for ( conmap = ( *mapI )->maincontainers.begin(); conmap != ( *mapI )->maincontainers.end(); ++conmap ) {
					// Containerinhalt altern
					for ( cmi = ( *conmap ).second.begin(); cmi != ( *conmap ).second.end(); ++cmi ) {
						if ( ( *cmi ).second != NULL ) {
							( *cmi ).second->doAge( AgeItem );
						}
					}
				}
			}
		} else {
			return false;
		}
	}
	// noch nicht die gesamte Karte durchlaufen ->
	// restliche Streifen bearbeiten
	lastXtoage = nextXtoage + ammount - 1;
	if ( lastXtoage >= maps.getHighX() ) {
		lastXtoage = maps.getHighX();
	}

	std::vector < CMap* > mapsToage;
	if ( maps.findAllMapsWithXInRangeOf( nextXtoage, lastXtoage, mapsToage ) ) {
		for ( std::vector < CMap* > ::iterator mapI = mapsToage.begin(); mapI < mapsToage.end(); ++mapI ) {
			( *mapI )->DoAgeItems_XFromTo( nextXtoage, lastXtoage, AgeItem );
		}
		closeShowcasesForContainerPositions();
		contpos->clear();
	}

	nextXtoage = lastXtoage + 1;

	return true;

}


void CWorld::AgeInventory( ITEM_FUNCT funct ) {

	PLAYERVECTOR::iterator titerator;
	for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
		( *titerator )->AgeInventory( funct );
	}

	MONSTERVECTOR::iterator monsterIterator;
	for ( monsterIterator = Monsters.begin(); monsterIterator < Monsters.end(); ++monsterIterator ) {
		( *monsterIterator )->AgeInventory( funct );
	}

}


void CWorld::AgeCharacters() {

	PLAYERVECTOR::iterator titerator;
	for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
		if (!( (*titerator )-> IsAlive() )) {
			( *titerator )->ReduceSkills();
		}
	}
}


void CWorld::saveAllPlayerNamesToFile( std::string name ) {

	PLAYERVECTOR::iterator titerator;
	FILE* f;
	f = fopen( name.c_str(), "w" );

	if ( f != NULL ) {
		std::list<std::string> players;

		for ( titerator = Players.begin(); titerator < Players.end(); ++titerator ) {
			if (!( *titerator )->isAdmin() || (*titerator)->hasGMRight(gmr_isnotshownasgm) )
				players.push_back(( *titerator )->name);
		}
		fprintf( f, "%d\n", (int)players.size());
		for (std::list<std::string>::iterator it = players.begin(); it != players.end(); it++)
			fprintf( f, "%s\n",  it->c_str() );
		fclose( f );
	}

}


void CWorld::Save( std::string prefix ) {

	char mname[ 200 ];
	prefix = directory + std::string( MAPDIR ) + prefix;

	std::ofstream mapinitfile( ( prefix + "_initmaps" ).c_str(), std::ios::binary | std::ios::out | std::ios::trunc );

	if ( ! mapinitfile.good() ) {
#ifdef CWorld_DEBUG
		std::cerr << "CWorld::Save: Fehler beim Speichern der Karten, konnte initmaps nicht erstellen" << std::endl;
#endif

	} else {
		unsigned short int size = maps.size();
		std::cout << "CWorld::Save: speichere " << size << " Karten" << std::endl;
		mapinitfile.write( ( char* ) & size, sizeof( size ) );

		for ( CMapVector::iterator mapI = maps.begin(); mapI != maps.end(); ++mapI ) {
			mapinitfile.write( ( char* ) & ( *mapI )->Z_Level, sizeof( ( *mapI )->Z_Level ) );
			mapinitfile.write( ( char* ) & ( *mapI )->Min_X, sizeof( ( *mapI )->Min_X ) );
			mapinitfile.write( ( char* ) & ( *mapI )->Min_Y, sizeof( ( *mapI )->Min_Y ) );

			mapinitfile.write( ( char* ) & ( *mapI )->Width, sizeof( ( *mapI )->Width ) );
			mapinitfile.write( ( char* ) & ( *mapI )->Height, sizeof( ( *mapI )->Height ) );

			mapinitfile.write( ( char* ) & ( *mapI )->disappears, sizeof( ( *mapI )->disappears ) );

			sprintf ( mname, "%s_%6d_%6d_%6d", prefix.c_str(), ( *mapI )->Z_Level, ( *mapI )->Min_X, ( *mapI )->Min_Y );
			( *mapI )->Save( mname );
		}
		mapinitfile.close();
	}


	std::ofstream specialfile( ( prefix + "_specialfields" ).c_str(), std::ios::binary | std::ios::out | std::ios::trunc );

	if ( ! specialfile.good() ) {
#ifdef CWorld_DEBUG
		std::cerr << "CWorld::Save: Fehler beim Speichern der speziellen Felder, konnte _specialfields nicht erstellen" << std::endl;
#endif

	} else {
		unsigned short int size = specialfields.size();
#ifdef CWorld_DEBUG
		std::cout << "CWorld::Save: speichere " << size << " spezielle Felder" << std::endl;
#endif
		specialfile.write( ( char* ) & size, sizeof( size ) );

		FIELDATTRIBHASH::const_iterator witerat = specialfields.begin();
		for ( ; witerat != specialfields.end(); ++witerat ) {
			specialfile.write( ( char* ) & ( witerat->first ), sizeof( witerat->first ) );
			specialfile.write( ( char* ) & ( witerat->second.type ), sizeof( witerat->second.type ) );
			specialfile.write( ( char* ) & ( witerat->second.flags ), sizeof( witerat->second.flags ) );
		}
		specialfile.close();
	}

}


void CWorld::Load( std::string prefix ) {
	char mname[ 200 ];
	prefix = directory + std::string( MAPDIR ) + prefix;

	std::ifstream mapinitfile( ( prefix + "_initmaps" ).c_str(), std::ios::binary | std::ios::in );

	if ( ! mapinitfile.good() ) {
		std::cerr << "CWorld::Load: Fehler beim Laden der Karten, konnte initmaps nicht öffnen" << std::endl;
	} else {
		unsigned short int size;
		mapinitfile.read( ( char* ) & size, sizeof( size ) );
		std::cout << "CWorld::Load: lade " << size << " Karten" << std::endl;

		short int tZ_Level;
		short int tMin_X;
		short int tMin_Y;

		short int tWidth;
		short int tHeight;

		bool tdisappears;

		CMap* tempMap;

		for ( int i = 0; i < size; ++i ) {
			mapinitfile.read( ( char* ) & tZ_Level, sizeof( tZ_Level ) );
			mapinitfile.read( ( char* ) & tMin_X, sizeof( tMin_X ) );
			mapinitfile.read( ( char* ) & tMin_Y, sizeof( tMin_Y ) );

			mapinitfile.read( ( char* ) & tWidth, sizeof( tWidth ) );
			mapinitfile.read( ( char* ) & tHeight, sizeof( tHeight ) );

			mapinitfile.read( ( char* ) & tdisappears, sizeof( tdisappears ) );

			tempMap = new CMap( tWidth, tHeight );
			tempMap->Init( tMin_X, tMin_Y, tZ_Level, tdisappears );

			sprintf ( mname, "%s_%6d_%6d_%6d", prefix.c_str(), tZ_Level, tMin_X, tMin_Y );
			// if the map loads ok...
			if (tempMap->Load( mname, 0, 0 ))
				maps.InsertMap( tempMap ); // insert it
		}
		mapinitfile.close();
	}

	std::ifstream specialfile( ( prefix + "_specialfields" ).c_str(), std::ios::binary | std::ios::in );
	if ( ! specialfile.good() ) {
		std::cerr << "CWorld::Load: Fehler beim Laden der speziellen Felder, konnte _specialfields nicht öffnen" << std::endl;
	} else {
		unsigned short int size3;
		specialfile.read( ( char* ) & size3, sizeof( size3 ) );
		std::cout << "CWorld::Load: lade " << size3 << " spezielle Felder" << std::endl;

		position start;
		s_fieldattrib attrib;

		for ( int i = 0 ; i < size3; ++i ) {
			specialfile.read( ( char* ) & start, sizeof( start ) );
			specialfile.read( ( char* ) & ( attrib.type ), sizeof( attrib.type ) );
			specialfile.read( ( char* ) & ( attrib.flags ), sizeof( attrib.flags ) );

			makeSpecialField( start, attrib );
		}
		specialfile.close();
	}

}

int CWorld::getTime(std::string timeType) {
    int minute,hour,day,month,year,illaTime;
    time_t curr_unixtime;
    struct tm *timestamp;

    // return unix timestamp if requsted and quit function
    if (timeType=="unix") return (int)time(NULL);

    // get current time and timezone data to get additional informations for time conversation
    curr_unixtime = time(NULL);
    timestamp = localtime( &curr_unixtime );

    illaTime = (int)curr_unixtime;

    // in case its currently dst, correct the timestamp so the illarion time changes the timestamp as well
    if (timestamp->tm_isdst)
        illaTime+=3600;

    // Illarion seconds since 17th February 2000
    // RL Seconds * 3
    illaTime = (illaTime - 950742000) * 3;

    if ( timeType=="illarion")return (int)illaTime;
    // Calculating year
    // 31536000 == 60*60*24*365
    year = (int)(illaTime / 31536000);
    illaTime -= year * 31536000;

    // Calculating day
    // 86400 = 60*60*24
    day = (int)(illaTime / 86400);
    illaTime -= day * 86400;
    ++day;

    // Calculating month
    // 24 days per month
    month = (int)(day / 24);
    day -= month * 24;
        
    // checking for range borders and fixing the date
    if (day == 0) {
        if (month > 0 && month < 16) {
            day = 24;
        } else {
            day = 5;
        }
    } else {
        month++;
    }

    if (month == 0) {
        month = 16;
        --year;
    }
    
    // Date calculation is done, return the date if it was requested
    if (timeType=="year") return year;
    else if (timeType=="month") return month;
    else if (timeType=="day") return day;

    // Calculate the time of day
    // Calculating hour
    // 3600 = 60 * 60
    hour = (int)(illaTime / 3600);
    illaTime -= hour * 3600;

    //Calculating minute
    minute = (int)(illaTime / 60);

    //Calculating seconds
    illaTime -= minute * 60;

    // returning the last possible values
    if (timeType=="hour") return hour;
    else if (timeType=="minute") return minute;
    else if (timeType=="second") return illaTime;
    else return -1;
}


bool CWorld::findWarpFieldsInRange( position pos, short int range, std::vector< boost::shared_ptr< position > > & warppositions ) {
	int x,y;
    CField * cf = 0;
    for( x=pos.x-range; x<=pos.x+range; ++x )
        for( y=pos.y-range; y<=pos.y+range; ++y )
            if( GetPToCFieldAt( cf, x, y, pos.z ) && cf->IsWarpField() )
            {
                boost::shared_ptr<position> p( new position( x, y, pos.z ) );
                warppositions.push_back( p );
            }
    return !warppositions.empty();
}


void CWorld::setWeatherPart(std::string type, char value)
{
    if ( type == "cloud_density" )
        weather.cloud_density = value;
    else if ( type == "fog_density" )
        weather.fog_density = value;
    else if ( type == "wind_dir" )
        weather.wind_dir = value;
    else if ( type == "gust_strength" )
        weather.gust_strength = value;
    else if ( type == "percipitation_strength" )
        weather.percipitation_strength = value;
    else if ( type == "percipitation_type" )
        weather.per_type = value;
    else if ( type == "thunderstorm" )
        weather.thunderstorm = value;
    else if ( type == "temperature" )
        weather.temperature = value;
    sendWeatherToAllPlayers();
}

void CWorld::sendRemoveCharToVisiblePlayers( TYPE_OF_CHARACTER_ID id, position & pos )
{
    std::vector < CPlayer* > temp = Players.findAllCharactersInRangeOf( pos.x, pos.y, pos.z, MAXVIEW );
    std::vector < CPlayer* > ::iterator titerator;
    boost::shared_ptr<CBasicServerCommand>cmd( new CRemoveCharTC( id ) );
    for ( titerator = temp.begin(); titerator < temp.end(); ++titerator )
    {
        ( *titerator )->sendCharRemove( id, cmd );
    }
}

