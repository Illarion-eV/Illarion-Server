#include "CField.hpp"

#include "CTilesModificatorTable.h"
#include "CCommonObjectTable.h"
#include "CContainerObjectTable.h"
#include "CTilesTable.h"
#include "globals.h"

//#define CField_DEBUG

CField::CField() {

	tile = 0;
	music = 0;
    clientflags = 0;
	extraflags = 0;
    warptarget = new position(0,0,0);
}

void CField::setTileId( unsigned short int id )
{
	tile = id;
}

unsigned short int CField::getTileCode()
{
	return tile;
}

unsigned short int CField::getTileId()
{
	if( ((tile & 0xFC00) >> 10) > 0 ) // shape exists
	{
		return tile & 0x001F;
	}
	else
	{
		return tile;
	}
}

void CField::setMusicId( unsigned short int id )
{
    music = id;
}

unsigned short int CField::getMusicId()
{
    return music;
}

ScriptItem CField::getStackItem( uint8_t spos )
{
    ScriptItem retItem;
    if ( items.empty() ) 
        return retItem;
    else
    {
        ITEMVECTOR::iterator theIterator;
        uint8_t counter = 0;
        for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator )
		{
            if ( counter >= spos ) 
            {
                retItem = (*theIterator);
                retItem.type = ScriptItem::it_field;
                retItem.itempos = counter;
                return retItem;
            }
            ++counter;
        }
        return retItem;
    }
}

CField::CField( const CField& source ) {

	tile = source.tile;
	clientflags = source.clientflags;
	extraflags = source.extraflags;
	items = source.items;
    warptarget = new position( *(source.warptarget) );
}


CField& CField:: operator =( const CField& source ) {

	if ( this != &source ) {
		tile = source.tile;
		clientflags = source.clientflags;
		extraflags = source.extraflags;
		items = source.items;
        warptarget->x = source.warptarget->x;
        warptarget->y = source.warptarget->y;
        warptarget->z = source.warptarget->z;
	}

	return *this;

}


CField::~CField()
{
    delete warptarget;
}


bool CField::addTopItem( const Item it ) {

	if ( IsPassable() ) {
		if ( items.size() < MAXITEMS ) {
			items.push_back( it );
			TilesModificatorStruct temp;
			if ( TilesModItems->find( it.id, temp ) ) {
				clientflags = clientflags | ( temp.Modificator & ( FLAG_GROUNDLEVEL ) );
				extraflags = extraflags | ( temp.Modificator & ( FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE ) );
			}
			return true;
		}
	}

	return false;

}


bool CField::PutGroundItem( const Item it ) {

	if ( items.size() < MAXITEMS ) {
		if ( items.empty() ) {
			items.push_back( it );
		} else {
			ITEMVECTOR::iterator iterat = items.begin();
			items.insert( iterat, it );
		}
		TilesModificatorStruct temp;
		if ( TilesModItems->find( it.id, temp ) ) {
			clientflags   = clientflags | ( temp.Modificator & ( FLAG_GROUNDLEVEL ) );
			extraflags   = extraflags | ( temp.Modificator & ( FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE ) );
		}
		return true;
	}

	return false;

}


bool CField::PutTopItem( const Item it ) {

	if ( items.size() < MAXITEMS ) {
		items.push_back( it );
		TilesModificatorStruct temp;
		if ( TilesModItems->find( it.id, temp ) ) {
			clientflags   = clientflags | ( temp.Modificator & ( FLAG_GROUNDLEVEL ) );
			extraflags   = extraflags | ( temp.Modificator & ( FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE ) );
		}
		return true;
	}

	return false;

}


bool CField::TakeTopItem( Item &it ) {

	if ( items.empty() ) {
		return false;
	}

	it = items.back();
	items.pop_back();
	updateFlags();

	return true;

}

bool CField::changeQualityOfTopItem(short int amount) {
	Item it;
	short int tmpQuality;
	if ( TakeTopItem( it ) ) {
		// don't increase the class of the item, but allow decrease of the item class
		tmpQuality = ((amount+it.quality%100)<100) ? (amount + it.quality) : (it.quality-it.quality%100 + 99);
		if ( tmpQuality%100 > 1) {
			it.quality = tmpQuality;
			PutTopItem(it);
			return false;
		} else {
			return true;
		}
	}
	return false;
}


int CField::increaseTopItem( int count, bool &erased ) {

	Item it;
	int temp = count;
#ifdef CField_DEBUG
	std::cout << "increaseTopItem " << count << "\n";
#endif
	if ( TakeTopItem( it ) ) {
		temp = count + it.number;
#ifdef CField_DEBUG
		std::cout << "temp " << temp << "\n";
#endif
#ifdef CField_DEBUG
		std::cout << "it.number:"<<it.number<<std::endl;
#endif
		if ( temp > MAXITEMS ) {
			it.number = MAXITEMS;
			temp = temp - MAXITEMS;
			PutTopItem( it );
			erased = false;
		} else if ( temp <= 0 ) {
			temp = count + it.number;
			erased = true;
		} else {
			it.number = temp;
			temp = 0;
			PutTopItem( it );
			erased = false;
		}
	}

	return temp;

}


bool CField::swapTopItem( TYPE_OF_ITEM_ID newid, uint16_t newQuality ) {

	std::cout << "swapTopItem " << std::endl;
	Item temp;
	if ( TakeTopItem( temp ) ) {
		std::cout << "old ID " << (int)temp.id << " old wear " << (int)temp.wear << std::endl;
		temp.id = newid;
		if ( newQuality > 0 ) temp.quality = newQuality;
		if ( CommonItems->find( newid, tempCommon ) ) {
			temp.wear=tempCommon.AgeingSpeed;
			std::cout << "new ID " << (int)temp.id << " new wear " << (int)temp.wear << std::endl;
		} else {
			std::cout << "new ID " << (int)temp.id << " new wear not found" << std::endl;
		}
		PutTopItem( temp );
		return true;
	}

	std::cout << "no item found to swap" << std::endl;

	return false;

}


bool CField::ViewTopItem( Item &it ) {

	if ( items.empty() ) {
		return false;
	}

	it = items.back();

	return true;

}


MAXCOUNTTYPE CField::NumberOfItems() {

	return items.size();

}


void CField::Save( std::ostream* mapt, std::ostream* obj, std::ostream* warp ) {

	mapt->write( ( char* ) & tile, sizeof( tile ) );
	mapt->write( ( char* ) & music, sizeof( music ) );
    mapt->write( ( char* ) & clientflags, sizeof( clientflags ) );
	mapt->write( ( char* ) & extraflags, sizeof( extraflags ) );

	ITEMVECTOR::iterator theIterator;

	unsigned char size = items.size();
	obj->write( ( char* ) & size, sizeof( size ) );

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) 
	{
		theIterator->save(obj);
	}

    if( IsWarpField() )
    {
        char b = 1;
        warp->write( ( char* ) & b, sizeof( b ) );
        warp->write( ( char* ) & warptarget->x, sizeof( warptarget->x ) );
        warp->write( ( char* ) & warptarget->y, sizeof( warptarget->x ) );
        warp->write( ( char* ) & warptarget->z, sizeof( warptarget->x ) );
    }
    else
    {
        char b = 0;
        warp->write( ( char* ) & b, sizeof( b ) );
    }
}


void CField::giveNonPassableItems( ITEMVECTOR &nonpassitems ) {

	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) {
		if ( TilesModItems->nonPassable( theIterator->id ) ) {
			nonpassitems.push_back( *theIterator );
		}
	}

}


void CField::giveExportItems( ITEMVECTOR &nonmoveitems ) {

	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) {
		if ( theIterator->wear == 255 ) {
			nonmoveitems.push_back( *theIterator );
		} else {
            if ( CommonItems->find( theIterator->id, tempCommon ) && tempCommon.AfterInfiniteRot > 0 )
            {
                Item rottedItem = *theIterator;
                rottedItem.id = tempCommon.AfterInfiniteRot;
                rottedItem.wear = 255;
                nonmoveitems.push_back( rottedItem );
            }
        }
	}
}


void CField::Load( std::istream* mapt, std::istream* obj, std::istream* warp ) {

	mapt->read( ( char* ) & tile, sizeof( tile ) );
	mapt->read( ( char* ) & music, sizeof( music ) );
    mapt->read( ( char* ) & clientflags, sizeof( clientflags ) );
	mapt->read( ( char* ) & extraflags, sizeof( extraflags ) );

	unsigned char ftemp = 255 - FLAG_NPCONFIELD - FLAG_MONSTERONFIELD - FLAG_PLAYERONFIELD;

	clientflags = clientflags & ftemp;

	MAXCOUNTTYPE size;
	obj->read( ( char* ) & size, sizeof( size ) );

	items.clear();
    Item temp;
	for ( int i = 0; i < size; ++i ) 
	{
		temp.load(obj);
        items.push_back( temp );
	}

    char iswarp = 0;
    warp->read( ( char* ) & iswarp, sizeof( iswarp ) );
    if( iswarp == 1 )
    {
        short int x, y, z;
        warp->read( ( char* ) & x, sizeof( warptarget->x ) );
        warp->read( ( char* ) & y, sizeof( warptarget->y ) );
        warp->read( ( char* ) & z, sizeof( warptarget->z ) );
        SetWarpField( position( x, y, z ) ); 
    }

}

int8_t CField::DoAgeItems( ITEM_FUNCT funct ) {

	int8_t ret = 0;
	if ( ! items.empty() ) {
		ITEMVECTOR::iterator theIterator = items.begin();
		while ( theIterator < items.end() ) 
        {
			if ( ! funct( &(*theIterator) ) ) 
            {
				if ( !CommonItems->find( theIterator->id, tempCommon ) ) 
                {
					tempCommon.ObjectAfterRot = theIterator->id;
				}

				if ( theIterator->id != tempCommon.ObjectAfterRot ) 
                {
#ifdef CField_DEBUG
					std::cout << "CFIELD:Ein Item wird umgewandelt von: " << theIterator->id << "  nach: " << tempCommon.ObjectAfterRot << "!\n";
#endif
					
                    //only set ret to 1 if it wasn't -1 because -1 has the highest priority (forces update of the field and rots container)
                    if ( theIterator->id != tempCommon.ObjectAfterRot && ret != -1)ret = 1;
		                        theIterator->id = tempCommon.ObjectAfterRot;
					if ( CommonItems->find( tempCommon.ObjectAfterRot, tempCommon ) ) 
                    {
						theIterator->wear = tempCommon.AgeingSpeed;
					}
					theIterator++;
				} 
                else 
                {
#ifdef CField_DEBUG
					std::cout << "CFIELD:Ein Item wird gel�cht,ID:" << theIterator->id << "!\n";
#endif

					if ( ContainerItems->find( theIterator->id ) ) 
                    {
						std::cout << "Ein Container auf einem Feld soll gel�cht werden!" << std::endl;
						erasedcontainers->push_back(theIterator->number);
						ret = -1;
					}

					theIterator = items.erase( theIterator );
                    if ( ret != -1 ) ret = 1;

				}
			} 
            else 
            {
				++theIterator;
			}
		}
	}
	updateFlags();

	return ret;

}

void CField::updateFlags() {

	// alle durch Items und Tiles modifizierte Flags l�chen
	clientflags = clientflags & ( 255 - ( FLAG_GROUNDLEVEL ) );
	extraflags = extraflags & ( 255 - ( FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE ) );

	TilesStruct tt;
	if ( Tiles->find( tile, tt ) ) {
		clientflags = clientflags | ( tt.flags & ( FLAG_GROUNDLEVEL ) );
		extraflags = extraflags | ( tt.flags & ( FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_SPECIALTILE + FLAG_MAKEPASSABLE ) );
	}

	TilesModificatorStruct tmod;
	ITEMVECTOR::iterator theIterator;
	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) {
		if ( TilesModItems->find( theIterator->id, tmod ) ) {
			clientflags   = clientflags | ( tmod.Modificator & ( FLAG_GROUNDLEVEL ) );
			extraflags   = extraflags | ( tmod.Modificator & ( FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE ) );
		}
	}

}


void CField::DeleteAllItems() {

	items.clear();
	updateFlags();

}


unsigned char CField::GroundLevel() {

	return ( clientflags & FLAG_GROUNDLEVEL );

}


bool CField::IsMonsterOnField() {

	return ( ( clientflags & FLAG_MONSTERONFIELD ) != 0 );

}


void CField::SetMonsterOnField( bool t ) {

	if ( t ) {
		clientflags = clientflags | FLAG_MONSTERONFIELD;
	} else {
		clientflags = clientflags & ( 255 - FLAG_MONSTERONFIELD );
	}

}


bool CField::IsNPCOnField() {

	return ( ( clientflags & FLAG_NPCONFIELD ) != 0 );

}


void CField::SetNPCOnField( bool t ) {

	if ( t ) {
		clientflags = clientflags | FLAG_NPCONFIELD;
	} else {
		clientflags = clientflags & ( 255 - FLAG_NPCONFIELD );
	}

}


bool CField::IsPlayerOnField() {

	return ( ( clientflags & FLAG_PLAYERONFIELD ) != 0 );

}


void CField::SetPlayerOnField( bool t ) {

	if ( t ) {
		clientflags = clientflags | FLAG_PLAYERONFIELD;
	} else {
		clientflags = clientflags & ( 255 - FLAG_PLAYERONFIELD );
	}

}



void CField::SetLevel( unsigned char z ) {

	z <<= 4;     // bits an die richtige Position bringen
	z &= FLAG_MAPLEVEL;     // andere bits l�chen
	clientflags &= ( 255 - FLAG_MAPLEVEL );     // maplevel - bits l�chen
	clientflags |= z;     // maplevel - bits setzen

}


bool CField::IsWarpField() {

	return ( ( extraflags & FLAG_WARPFIELD ) != 0 );

}



void CField::SetWarpField( const position & pos )
{
    warptarget->x = pos.x;
    warptarget->y = pos.y;
    warptarget->z = pos.z;
    extraflags = extraflags | FLAG_WARPFIELD;
}


void CField::UnsetWarpField()
{
    extraflags = extraflags & ( 255 - FLAG_WARPFIELD );
}


void CField::GetWarpField( position & pos )
{
    pos.x = warptarget->x;
    pos.y = warptarget->y;
    pos.z = warptarget->z;
}


bool CField::HasSpecialItem() {

	return ( ( extraflags & FLAG_SPECIALITEM ) != 0 );

}


void CField::SetSpecialItem( bool t ) {

	if ( t ) {
		extraflags = extraflags | FLAG_SPECIALITEM;
	} else {
		extraflags = extraflags & ( 255 - FLAG_SPECIALITEM );
	}

}


bool CField::IsSpecialField() {

	return ( ( extraflags & FLAG_SPECIALTILE ) != 0 );

}


void CField::SetSpecialField( bool t ) {

	if ( t ) {
		extraflags = extraflags | FLAG_SPECIALTILE;
	} else {
		extraflags = extraflags & ( 255 - FLAG_SPECIALTILE );
	}

}


bool CField::IsTransparent() {

	return ( ( extraflags & FLAG_TRANSPARENT ) == 0 );

}


bool CField::IsPassable() const {

	return ( ( ( extraflags & FLAG_PASSABLE ) == 0 ) || ( ( extraflags & FLAG_MAKEPASSABLE ) != 0 ) );

}


bool CField::IsPenetrateable() {

	return ( ( extraflags & FLAG_PENETRATEABLE ) == 0 );

}


bool CField::moveToPossible() const {
	return (
			   ( IsPassable()  &&
				 ( ( clientflags & ( FLAG_MONSTERONFIELD | FLAG_NPCONFIELD | FLAG_PLAYERONFIELD ) ) == 0 ) )
		   );
}

void CField::setChar() {
	clientflags |= FLAG_PLAYERONFIELD;
}

void CField::removeChar() {
	clientflags &= ~FLAG_PLAYERONFIELD;
}
