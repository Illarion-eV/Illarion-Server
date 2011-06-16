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


#include "CWorld.hpp"
#include "CPlayer.hpp"
#include "Item.hpp"
#include "CCommonObjectTable.h"
#include "CField.hpp"
#include "CNamesObjectTable.h"
#include "CArmorObjectTable.h"
#include "CWeaponObjectTable.h"
#include "CNaturalArmorTable.hpp"
#include "netinterface/protocol/ServerCommands.hpp"
#include "netinterface/protocol/BBIWIServerCommands.hpp"
#include "fuse_ptr.hpp"

extern CCommonObjectTable * CommonItems;
extern CNamesObjectTable* ItemNames;
class CCharacter;

bool CWorld::deleteNPC( unsigned int npcid )
{   /*
	int posx,posy,posz;
	CField* tempf; //alte NPC's l?chen
	NPCVECTOR::iterator npcIterator;
	for ( npcIterator = Npc.begin(); npcIterator < Npc.end(); ++npcIterator ) {
	    if (( *npcIterator )->id == npcid) {
    		if ( GetPToCFieldAt( tempf, ( *npcIterator )->pos.x, ( *npcIterator )->pos.y, ( *npcIterator )->pos.z ) ) {
    			//tempf->SetNPCOnField( false );
    			tempf->removeChar();
    		}
    		
            sendRemoveCharToVisiblePlayers( ( *npcIterator )->id, ( *npcIterator )->pos );
    		delete( *npcIterator );
    		Npc.erase( npcIterator );
    		return true;
    	}
	}
	return false;*/
    LostNpcs.push_back(npcid);
    return true;
}

bool CWorld::createDynamicNPC( std::string name, CCharacter::race_type type, position pos, /*CCharacter::face_to dir,*/ CCharacter::sex_type sex, std::string scriptname)
{
    try {
        
		try {
			CNPC* newNPC = new CNPC(DYNNPC_BASE, name, type, pos, (CCharacter::face_to)4/*dir*/, false, sex, 0, 0, 255, 255, 255, 255, 255, 255);
			
			// add npc to npc list
			Npc.push_back(newNPC);
			
			// set field to occupied
			CField* tempf;
			if ( GetPToCFieldAt( tempf, pos ) ) {
				tempf->setChar();
			}

			try {
				// try to load the script
				boost::shared_ptr<CLuaNPCScript> script( new CLuaNPCScript( scriptname, newNPC ) );
				newNPC->setScript(script);
			} catch (ScriptException &e) {
				std::cerr << "CWorld::createDynamicNPC: Error while loading dynamic NPC script: " << scriptname << " : " << e.what() << std::endl;
			}
		} catch (NoSpace &s) {
			std::cerr << "CWorld::createDynamicNPC: No space available for dynamic NPC: " << name << " : " << s.what() << std::endl;
		}

		return true;
	} catch (...) {
        std::cerr << "CWorld::createDynamicNPC: Unknown error while loading dynamic NPC: " << name << std::endl;
		return false;
	}
}

luabind::object CWorld::LuaLoS(position startingpos, position endingpos)
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;
    std::list<BlockingObject> objects = LoS(startingpos, endingpos);
    for ( std::list<BlockingObject>::iterator boIterator = objects.begin(); boIterator != objects.end(); boIterator++ )
    {
        luabind::object innerlist = luabind::newtable( luaState );
        if ( boIterator->blockingType == BlockingObject::BT_CHARACTER )
        {
            innerlist["TYPE"] = "CHARACTER";
            innerlist["OBJECT"] = fuse_ptr<CCharacter>(boIterator->blockingChar);
        }
        else if ( boIterator->blockingType == BlockingObject::BT_ITEM )
        {
            innerlist["TYPE"] = "ITEM";
            innerlist["OBJECT"] = boIterator->blockingItem;
        }
        list[index] = innerlist;
        index++;
    }
    return list;
}

luabind::object CWorld::getPlayersOnline()
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;
    PLAYERVECTOR::iterator pIterator;
    for ( pIterator = Players.begin(); pIterator != Players.end(); ++pIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*pIterator);
        index++;
    }
    return list;
}

luabind::object CWorld::getNPCS()
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;
    NPCVECTOR::iterator npcIterator;
    for ( npcIterator = Npc.begin(); npcIterator != Npc.end(); ++npcIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*npcIterator);
        index++;
    }
    return list;

}

luabind::object CWorld::getCharactersInRangeOf( position posi, uint8_t range)
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;

	std::vector < CPlayer* > tempP = Players.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CPlayer*>::iterator pIterator = tempP.begin(); pIterator != tempP.end() ; ++pIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*pIterator);
        index++;
    }
    
	std::vector < CMonster* > tempM = Monsters.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CMonster*>::iterator mIterator = tempM.begin(); mIterator != tempM.end() ; ++mIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*mIterator);
        index++;
    }

	std::vector < CNPC* > tempN = Npc.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CNPC*>::iterator nIterator = tempN.begin(); nIterator != tempN.end() ; ++nIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*nIterator);
        index++;
    }
    return list;
}

luabind::object CWorld::getPlayersInRangeOf( position posi, uint8_t range )
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;

	std::vector < CPlayer* > tempP = Players.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CPlayer*>::iterator pIterator = tempP.begin(); pIterator != tempP.end() ; ++pIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*pIterator);
        index++;
    }
    return list;
}

luabind::object CWorld::getMonstersInRangeOf( position posi, uint8_t range )
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;

	std::vector < CMonster* > tempM = Monsters.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CMonster*>::iterator mIterator = tempM.begin(); mIterator != tempM.end() ; ++mIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*mIterator);
        index++;
    }
    return list;
}

luabind::object CWorld::getNPCSInRangeOf( position posi, uint8_t range )
{
    lua_State* luaState = getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable( luaState );
    int index = 1;

	std::vector < CNPC* > tempN = Npc.findAllCharactersInRangeOf(posi.x , posi.y, posi.z, range );
	for ( std::vector< CNPC*>::iterator nIterator = tempN.begin(); nIterator != tempN.end() ; ++nIterator )
    {
        list[index] = fuse_ptr<CCharacter>(*nIterator);
        index++;
    }
    return list;
}

void CWorld::ItemInform(CCharacter * user, ScriptItem item, std::string message) 
{
	if ( user->character != CCharacter::player ) 
    {
		return;
	}
	CPlayer * cp = dynamic_cast<CPlayer*>(user);
	if ( item.type == ScriptItem::it_showcase1 ||item.type == ScriptItem::it_showcase2 ) 
        {
		if ( item.owner->character == CCharacter::player ) 
        {
			unsigned char showcase;
			if ( item.type == ScriptItem::it_showcase1 ) showcase = 0;
			else if ( item.type == ScriptItem::it_showcase2 ) showcase = 1;
            boost::shared_ptr<CBasicServerCommand>cmd(new CNameOfShowCaseItemTC( showcase, item.itempos, message) );
			cp->Connection->addCommand( cmd );
		}
	} 
    else if ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt ) 
    {
		if ( item.owner->character == CCharacter::player ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd(new CNameOfInventoryItemTC( item.itempos, message) );
			cp->Connection->addCommand( cmd );
		}
	} 
    else if ( item.type == ScriptItem::it_field ) 
    {
		if ( item.owner->character == CCharacter::player ) 
        {
            boost::shared_ptr<CBasicServerCommand>cmd(new CNameOfMapItemTC( item.pos.x, item.pos.y, item.pos.z , message) );
			cp->Connection->addCommand( cmd );
		}
	}
}


void CWorld::changeQuality( ScriptItem item, short int amount ) {
	short int tmpQuality = ((amount+item.quality%100)<100) ? (amount + item.quality) : (item.quality-item.quality%100 + 99);
	if ( tmpQuality%100 > 0) {
		item.quality = tmpQuality;
		changeItem(item);
	} else {
		erase(item,MAXITEMS);
	}
}

void CWorld::changeQualityOfItemAt( position pos, short int amount) {
	CField * field;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z) ) {
		if ( field->changeQualityOfTopItem(amount) ) {
			sendRemoveItemFromMapToAllVisibleCharacters( 0, pos.x, pos.y, pos.z, field );
		}
	}
}

bool CWorld::changeItem(ScriptItem item) 
{

		if ( item.type == ScriptItem::it_showcase1 || item.type == ScriptItem::it_showcase2 ) 
        {
			if ( item.owner->character == CCharacter::player ) 
            {
				CContainer * showcase;
				if ( item.type == ScriptItem::it_showcase1)
					showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 0 ].top();
				else if ( item.type == ScriptItem::it_showcase2)
					showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 1 ].top();
				//Neues Item an der Position erzeugen
				showcase->changeItem( item );
				sendChangesOfContainerContentsIM( showcase );
				return true;
			} 
            else 
            {
				return false;
			}
		} 
        else if ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt ) 
        {
            item.owner->characterItems[ item.itempos ].id = item.id;
			item.owner->characterItems[ item.itempos ].wear = item.wear;
			item.owner->characterItems[ item.itempos ].number = item.number;
			item.owner->characterItems[ item.itempos ].quality = item.quality;
            item.owner->characterItems[ item.itempos ].data = item.data;
            item.owner->characterItems[ item.itempos ].data_map = item.data_map;
      		//Wenn character ein Spieler ist ein update schicken
			if ( item.owner->character == CCharacter::player )
				dynamic_cast<CPlayer*>(item.owner)->sendCharacterItemAtPos(item.itempos);
            item.owner->updateAppearanceForAll( true );
			return true;
		} 
        else if ( item.type == ScriptItem::it_field ) 
        {
			CField *field;
			Item dummy;
			if ( GetPToCFieldAt(field, item.pos.x, item.pos.y, item.pos.z) ) 
            {
                Item it;
                if ( field->TakeTopItem( it ) )
                {
                    field->PutTopItem( static_cast<Item>(item) );
                    if ( item.id != it.id || it.number != item.number ) 
                    {
                        sendSwapItemOnMapToAllVisibleCharacter( it.id, item.pos.x, item.pos.y, item.pos.z, item, field );
                    }
                }
                return true;
			} 
            else 
            {
				return false;
			}
		}
        else if ( item.type == ScriptItem::it_container )
        {
            if ( item.inside )
            {
                item.inside->changeItem(item);
                sendChangesOfContainerContentsIM( item.inside );
                return true;
            }
        }
	return false;
}

std::string CWorld::getItemName(TYPE_OF_ITEM_ID itemid, uint8_t language)
{
    NamesStruct name;
    if ( ItemNames->find(itemid, name) )
    {
        if ( language == 0 ) return name.German;
        else return name.English;
    }    
    return "notfound";
}


CommonStruct CWorld::getItemStats(ScriptItem item) {
	CommonStruct data;
	if ( CommonItems->find(item.id,data ) ) {
		return data;
	} else {
		data.id = 0;
		return data;
	}
}

CommonStruct CWorld::getItemStatsFromId(TYPE_OF_ITEM_ID id) {
	CommonStruct data;
	if ( CommonItems->find(id,data ) ) {
		return data;
	} else {
		data.id = 0;
		return data;
	}
}

bool CWorld::isCharacterOnField(position pos)
{
	if ( findCharacterOnField(pos.x, pos.y, pos.z) ) return true;
	else return false;
}

fuse_ptr<CCharacter> CWorld::getCharacterOnField(position pos) 
{
	return fuse_ptr<CCharacter>( findCharacterOnField(pos.x, pos.y, pos.z) );
}

bool CWorld::erase(ScriptItem item, int amount) 
{
	/*if (amount==0) {
		if ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt || item.type == ScriptItem::it_field )
			amount=item.number;
		else
			amount=showcase->increaseAtPos(item.itempos,0);
	}*/
    if ( amount > item.number ) amount = item.number;

	//unsigned char amount=1;

	//Item befindet sich in einen der beiden Showcases
	if ( item.type == ScriptItem::it_showcase1 || item.type == ScriptItem::it_showcase2 ) 
    {
		if ( item.owner->character == CCharacter::player ) 
        {
			CContainer * showcase;
			if (item.type == ScriptItem::it_showcase1)
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 0 ].top();
			else
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 1 ].top();
			showcase->increaseAtPos( item.itempos, -amount );
			sendChangesOfContainerContentsIM( showcase );

			return true;
		} 
        else 
        {
			return false;
		}
	}
	//Item befindet sich am Körper oder im Gürtel
	else if 
    ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt ) 
    {
		//Wenn Item rechts belegt und links ein Belegt ist [Zweihanditem] das Belegt mit löschen
		if ( item.itempos == RIGHT_TOOL && (item.owner->GetItemAt(LEFT_TOOL)).id == BLOCKEDITEM ) item.owner->increaseAtPos( LEFT_TOOL, -255);
        else if ( item.itempos == LEFT_TOOL && (item.owner->GetItemAt(RIGHT_TOOL)).id == BLOCKEDITEM ) item.owner->increaseAtPos( RIGHT_TOOL, -255);
        item.owner->increaseAtPos( item.itempos, -amount );
		return true;
	}
	//Item befindet sich auf einen Feld am Boden liegend.
	else if ( item.type == ScriptItem::it_field ) 
    {
		CField *field;
		Item dummy;
		if ( GetPToCFieldAt(field, item.pos.x, item.pos.y, item.pos.z) ) 
        {
			bool erased=false;
			field->increaseTopItem( -amount, erased );
			if ( erased )
				sendRemoveItemFromMapToAllVisibleCharacters( 0, item.pos.x, item.pos.y, item.pos.z, field );
			return true;
		} 
        else 
        {
			std::cerr<<"CWorld::erase: Field ("<<item.pos.x<<", "<<item.pos.y<<", "<<item.pos.z<<") was not found!"<<std::endl;
			return false;
		}
	}
    else if ( item.type == ScriptItem::it_container )
    {
        if ( item.inside )
        {
            item.inside->increaseAtPos( item.itempos, -amount);
            sendChangesOfContainerContentsIM( item.inside );
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}


bool CWorld::increase(ScriptItem item, short int count) {
	//Item befindet sich in einen der beiden Showcases
	if ( item.type == ScriptItem::it_showcase1 || item.type == ScriptItem::it_showcase2 ) {
		if ( item.owner->character == CCharacter::player ) {
			CContainer * showcase;
			if (item.type == ScriptItem::it_showcase1)
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 0 ].top();
			else
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 1 ].top();
			showcase->increaseAtPos( item.itempos, count );
			sendChangesOfContainerContentsIM( showcase );
			return true;
		} else {
			return false;
		}
	}
	//Item befindet sich am Körper oder im Rucksack
	else if ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt ) {
		item.owner->increaseAtPos( item.itempos, count );
		return true;
	}
	//Item befindet sich auf einen Feld am Boden liegend.
	else if ( item.type == ScriptItem::it_field ) {
		CField *field;
		if ( GetPToCFieldAt(field, item.pos.x, item.pos.y, item.pos.z) ) {
			bool erased=false;
			field->increaseTopItem( count, erased );
			if ( erased )
				sendRemoveItemFromMapToAllVisibleCharacters( 0, item.pos.x, item.pos.y, item.pos.z, field );
			return true;
		} else {
            std::cerr<<"CWorld::increase: Field ("<<item.pos.x<<", "<<item.pos.y<<", "<<item.pos.z<<") was not found!"<<std::endl;
			return false;
		}
	}
    
    else if ( item.type == ScriptItem::it_container )
    {
        if ( item.inside )
        {
            item.inside->increaseAtPos( item.itempos, count);
            sendChangesOfContainerContentsIM( item.inside );
            return true;
        }
        else
        {
            return false;
        }
    }
	return false;
}

bool CWorld::swap(ScriptItem item, TYPE_OF_ITEM_ID newitem, unsigned short int newQuality) {
	//Item befindet sich in einen der beiden Showcases
	if ( item.type == ScriptItem::it_showcase1 || item.type == ScriptItem::it_showcase2 ) {
		if ( item.owner->character == CCharacter::player ) {
			CContainer * showcase;
			if (item.type == ScriptItem::it_showcase1)
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 0 ].top();
			else
				showcase = dynamic_cast<CPlayer*>(item.owner)->showcases[ 1 ].top();
			showcase->swapAtPos( item.itempos, newitem, newQuality );
			sendChangesOfContainerContentsIM( showcase );
			return true;
		} else {
			return false;
		}
	}
	//Item befindet sich am Körper oder im Rucksack
	else if ( item.type == ScriptItem::it_inventory || item.type == ScriptItem::it_belt ) {
		item.owner->swapAtPos( item.itempos, newitem, newQuality );
		return true;
	}
	//Item befindet sich auf einen Feld am Boden liegend.
	else if ( item.type == ScriptItem::it_field ) {
		CField *field;
		if ( GetPToCFieldAt(field, item.pos.x, item.pos.y, item.pos.z) ) {
			bool ok;
			Item it;
			if ( field->ViewTopItem( it ) ) {
				ok = field->swapTopItem( newitem, newQuality );
				if ( ok ) 
                {
                    Item dummy;
                    dummy.id = newitem;
                    dummy.number = it.number;
					if ( it.id != newitem ) sendSwapItemOnMapToAllVisibleCharacter( it.id, item.pos.x, item.pos.y, item.pos.z, dummy, field );
				} else {
                    std::cerr<<"CWorld::swap: Swapping item on Field ("<<item.pos.x<<", "<<item.pos.y<<", "<<item.pos.z<<") failed!"<<std::endl;
					return false;
				}
			}
		} else {
            std::cerr<<"CWorld::swap: Field ("<<item.pos.x<<", "<<item.pos.y<<", "<<item.pos.z<<") was not found!"<<std::endl;
			return false;
		}
	}
    
    else if ( item.type == ScriptItem::it_container )
    {
        if ( item.inside )
        {
            item.inside->swapAtPos( item.itempos, newitem, newQuality);
            sendChangesOfContainerContentsIM( item.inside );
            return true;
        }
        else
        {
            return false;
        }
    }
    
    
	return false;
}

ScriptItem CWorld::createFromId(TYPE_OF_ITEM_ID id, unsigned short int count, position pos, bool allways, int quali, int data) {
	CField * field;
    ScriptItem sItem;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z ) ) {
		CommonStruct com;
		if ( CommonItems->find(id, com) ) {
			g_item.id = id;
			g_item.number = count;
			g_item.wear = com.AgeingSpeed;
			g_item.quality = quali;
			g_item.data = data;
            g_item.setData(data);
			g_cont = NULL;
            sItem = g_item;
            sItem.pos = pos;
            sItem.type = ScriptItem::it_field;
            sItem.itempos = 255;
            sItem.owner = NULL;
            if ( allways )
				putItemAlwaysOnMap(NULL,pos.x,pos.y,pos.z);
			else
				putItemOnMap(NULL,pos.x,pos.y,pos.z);
			return sItem;
		} else {
			std::cerr<<"CWorld::createFromId: Item "<<id<<" was not found in CommonItems!"<<std::endl;
            return sItem;
		}
	} else {
        std::cerr<<"CWorld::createFromId: Field ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<") was not found!"<<std::endl;
		return sItem;
	}
	return sItem;

}

bool CWorld::createFromItem(ScriptItem item, position pos, bool allways) {
	CField * field;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z ) ) {
		g_item = static_cast<Item>(item);
		g_cont = NULL;
		if ( allways )
			putItemAlwaysOnMap(NULL,pos.x,pos.y,pos.z);
		else
			putItemOnMap(NULL,pos.x,pos.y,pos.z);
		return true;
	} else {
		std::cerr<<"CWorld::createFromItem: Field ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<") was not found!"<<std::endl;
		return false;
	}
	return false;
}

bool CWorld::createMonster(unsigned short id, position pos, short movepoints) {
	CField * field;
	CMonster * newMonster;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z ) ) {
		try {
			newMonster = new CMonster(id, pos);
#ifdef CLUASCRIPT_DEBUG
			std::cout<<"Erschaffe neues Monster: " << newMonster->name << " an Position (x,y,z) " << pos.x << " " << pos.y << " " << pos.z << std::endl;
#endif
			newMonster->actionPoints = movepoints;
			newMonsters.push_back( newMonster );
			field->setChar();
			sendCharacterMoveToAllVisiblePlayers( newMonster, NORMALMOVE, 4 );
			return true;

		} catch (CMonster::unknownIDException) {
			std::cerr << "CWorld::createMonster: Failed to create monster with unknown id (" << id << ")!" << std::endl;
			return false;
		}
	} else {
        std::cerr<<"CWorld::createMonster: Field ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<") was not found!"<<std::endl;
		return false;
	}
	return false;
}

void CWorld::gfx(unsigned short int gfxid, position pos) {
	makeGFXForAllPlayersInRange( pos.x, pos.y, pos.z, MAXVIEW, gfxid );
}

void CWorld::makeSound( unsigned short int soundid, position pos ) {
       makeSoundForAllPlayersInRange( pos.x, pos.y, pos.z, MAXVIEW, soundid );
}

bool CWorld::isItemOnField(position pos) {
	CField * field;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z) ) {
		Item dummy;
		return field->ViewTopItem( dummy );
	} else {
        std::cerr<<"CWorld::isItemOnField: Field ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<") was not found!"<<std::endl;
	}
	return false;
}

ScriptItem CWorld::getItemOnField(position pos) {
	CField * field;
	ScriptItem item;
	if ( GetPToCFieldAt( field, pos.x, pos.y, pos.z) ) {
		Item It;
		if ( field->ViewTopItem( It ) ) {
			item = It;
			item.pos = pos;
			item.type = ScriptItem::it_field;
			return item;
		}
	} else {
        std::cerr<<"CWorld::getItemOnField: Field ("<<pos.x<<", "<<pos.y<<", "<<pos.z<<") was not found!"<<std::endl;
	}
	return item;
}

void CWorld::changeTile(short int tileid, position pos)
{
    CField* field;
	if (GetPToCFieldAt(field, pos.x, pos.y, pos.z))
	{
		field->setTileId( tileid );
		field->updateFlags();
	}
}


void CWorld::sendMapUpdate(position pos, uint8_t range)
{
    std::vector<CPlayer*> temp;
	std::vector<CPlayer*>::iterator pIterator;
	temp=Players.findAllCharactersInRangeOf(pos.x,pos.y,pos.z, range );//getPlayersInRange(pos.x,pos.y,pos.z,MAXVIEW);
		
	for ( pIterator = temp.begin(); pIterator != temp.end(); ++pIterator ) 
	{
        (*pIterator)->sendFullMap();
		sendAllVisibleCharactersToPlayer( ( *pIterator ), true );
	}
}

bool CWorld::createSavedArea(uint16_t tileid, position pos, uint16_t height, uint16_t width)
{
    //Schleife durch alle spalten
    CMap* dummy;
    for ( time_t akt_x = pos.x; akt_x < pos.x+width; ++akt_x)
    {
        for ( time_t akt_y = pos.y; akt_y < pos.y+height; ++akt_y)
        {
                //Prüfen ob auf der position ein gültiges Feld ist, wenn ja false zurück liefern
                if ( maps.findMapForPos( akt_x, akt_y, pos.z, dummy ) )
                {
                    std::cerr<<"CWorld::createSavedArea: Aborted map insertion, map for field at ("<<akt_x <<", "<<akt_y<<", "<<pos.z<<") found!"<<std::endl;
                    return false;
                }
        }
    }
       
   	CMap* tempmap = new CMap(width,height);
	bool disappear=true;
	tempmap->Init(pos.x, pos.y, pos.z, disappear);

	CField* tempf;

	for (int _x=0; _x<width; ++_x)
		for (int _y=0; _y<height; ++_y) {
			if (tempmap->GetPToCFieldAt(tempf, _x+pos.x, _y+pos.y)) {
				tempf->setTileId( tileid );
				tempf->updateFlags();
			} else std::cerr << "CWorld::createSavedArea: For map inserted at (" << pos.x << ", " << pos.y << ", " << pos.z << ") no Field was found for offset (" << _x << ", " << _y << ")!" << std::endl;

		}

	maps.InsertMap(tempmap);
	std::cout<<" Map Created by createSavedArea command at x: "<<pos.x<<" y: "<<pos.y<<" z: "<<pos.z<<" height: "<<height<<" width: "<<width<<" standard tile: "<<tileid<<"!"<<std::endl;
	return true;
}


bool CWorld::getArmorStruct(TYPE_OF_ITEM_ID id, ArmorStruct &ret)
{
    //Has to be an own function cant give a pointer of Armor items to the script
    ArmorStruct as;
    if ( id == 0 )
    {
        ret = as;
        return false;
    }
    else
        return ArmorItems->find(id, ret);
    //return armor;
    
}

bool CWorld::getWeaponStruct(TYPE_OF_ITEM_ID id, WeaponStruct &ret)
{
    //Has to be an own function cant give a pointer of Armor items to the script
    WeaponStruct ws;
    if ( id == 0 )
    {
        ret = ws;
        return false;
    }
    else
        return WeaponItems->find(id, ret);
}

bool CWorld::getNaturalArmor(CCharacter::race_type id, MonsterArmor &ret)
{
    MonsterArmor ma;
    if ( id == 0 )
    {
        ret = ma;
        return false;
    }
    else
        return NaturalArmors->find(id, ret);
}

bool CWorld::getMonsterAttack(CCharacter::race_type id, AttackBoni &ret)
{
    return MonsterAttacks->find(id, ret);
}

void CWorld::sendMonitoringMessage(std::string msg, unsigned char id)
{
    //send this Command to all Monitoring Clients
    boost::shared_ptr<CBasicServerCommand>cmd(new CBBMessageTC(msg, id) );
    monitoringClientList->sendCommand(cmd);
}
