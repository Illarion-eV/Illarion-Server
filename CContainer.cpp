#include "CContainer.h"
#include "CContainerObjectTable.h"
#include "CCommonObjectTable.h"
#include "CWorld.hpp"

CContainer::CContainer(uint16_t max_cont_Volume) : max_Volume(max_cont_Volume) {
#ifdef CContainer_DEBUG
	std::cout << "CContainer Konstruktor Start/Ende" << std::endl;
#endif
}


CContainer::CContainer( const CContainer& source ) {
#ifdef CContainer_DEBUG
	std::cout << "CContainer Copy Konstruktor Start" << std::endl;
#endif
	*this = source;
#ifdef CContainer_DEBUG
	std::cout << "CContainer Copy Konstruktor Ende" << std::endl;
#endif
}


CContainer& CContainer:: operator =( const CContainer& source ) {
#ifdef CContainer_DEBUG
	std::cout << "CContainer Zuweisungsoperator Start" << std::endl;
#endif
	if ( this != &source ) {
		// alte Item löschen
		items.clear();
		// Item kopieren
		items = source.items;
		// alte CONTAINERMAP löschen (rekursiv alle Container)
		if ( !containers.empty() ) {
			CONTAINERMAP::iterator theIterator;
			for ( theIterator = containers.begin(); theIterator != containers.end(); ++theIterator ) {
				delete ( *theIterator ).second;
				( *theIterator ).second = NULL;
			}
			containers.clear();
		}

		// alle Container kopieren (rekursiv)
		if ( !source.containers.empty() ) {
			CONTAINERMAP::const_iterator theIterator;
			for ( theIterator = source.containers.begin(); theIterator != source.containers.end(); ++theIterator ) {
				containers.insert( CONTAINERMAP::value_type( ( *theIterator ).first, new CContainer( *( ( *theIterator ).second ) ) ) );
			}
		}

	}
#ifdef CContainer_DEBUG
	std::cout << "CContainer Zuweisungsoperator Ende" << std::endl;
#endif

	return *this;

}


CContainer::~CContainer() {
#ifdef CContainer_DEBUG
	std::cout << "CContainer Destruktor Start" << std::endl;
#endif
	items.clear();
	if ( !containers.empty() ) {
		CONTAINERMAP::iterator theIterator;
		for ( theIterator = containers.begin(); theIterator != containers.end(); ++theIterator ) {
			delete ( *theIterator ).second;
			( *theIterator ).second = NULL;
		}
		containers.clear();
	}
#ifdef CContainer_DEBUG
	std::cout << "CContainer Destruktor Ende" << std::endl;
#endif
}


bool CContainer::InsertItem( Item it ) {

#ifdef CContainer_DEBUG
	std::cout << "CContainer: altes InsertItem" << std::endl;
#endif

	return InsertItem( it, false);

}

bool CContainer::InsertItemOnLoad( Item it )
{
#ifdef CContainer_DEBUG
	std::cout << "CContainer: neues InsertItem" << std::endl;
#endif
	items.push_back( it );
	return false;    
}


bool CContainer::InsertItem( Item it, bool merge) {

#ifdef CContainer_DEBUG
	std::cout << "CContainer: neues InsertItem" << std::endl;
#endif
	ContainerStruct cont;
	if ( items.size() < MAXITEMS && VolOk(it) ) {
		if ( ContainerItems->find( it.id, cont ) ) {
			return InsertContainer( it, new CContainer( cont.ContainerVolume ) );
		} else if (merge) {
			//Unstackable von Items
			if ( isItemStackable(it) ) {
				ITEMVECTOR::iterator theIterator = items.begin();
				int temp;
				while ( ( theIterator < items.end() ) && ( it.number > 0 ) ) {
					if ( theIterator->id == it.id && theIterator->getData() == it.getData() && theIterator->quality >= 100 && it.quality >= 100 ) {
						temp = theIterator->number + it.number;
						if (temp <= MAXITEMS) {
							theIterator->number=temp;
							it.number=0;
						} else {
							it.number-=(MAXITEMS - theIterator->number);
							theIterator->number=MAXITEMS;
						}
                        if( it.quality < theIterator->quality )
                            theIterator->quality = it.quality;
					}
					theIterator++;
				}
			}
			if (it.number > 0) {
				items.push_back( it );
			}

			return true;
		} else {
			items.push_back( it );
			return true;
		}
	}

	return false;

}


bool CContainer::InsertItem( Item it, unsigned char pos) {
#ifdef CContainer_DEBUG
	std::cout << "CContainer: neues InsertItem mit pos" << std::endl;
#endif
	ContainerStruct cont;
	if ( items.size() < MAXCONTAINERITEMS && VolOk(it) ) {
		if ( ContainerItems->find( it.id, cont ) ) {
			return InsertContainer( it, new CContainer(cont.ContainerVolume) );
		}

		ITEMVECTOR::iterator theIterator;
		MAXCOUNTTYPE count2 = 0;
		theIterator = items.begin();
		// das Item Nummer pos in dem Vektor finden
		while ( ( theIterator < items.end() ) && ( count2 != pos ) ) {
			count2++;
			theIterator++;
		}
		//Itemstacking deaktivieren
		if ( isItemStackable(it) ) {
			if ( theIterator < items.end() ) {
				if ( theIterator->id == it.id && theIterator->getData() == it.getData() && theIterator->quality >= 100 && it.quality >= 100 ) {
					int temp = theIterator->number + it.number;
					if (temp <= MAXITEMS) {
						theIterator->number=temp;
						it.number=0;
					} else {
						it.number-=(MAXITEMS - theIterator->number);
						theIterator->number=MAXITEMS;
					}
                    if( it.quality < theIterator->quality )
                        theIterator->quality = it.quality;
				}
			}
		}
		//Ende Itemstacking deaktivieren.

		if (it.number > 0) {
			items.push_back( it );
		}
		return true;
	}

	return false;

}


bool CContainer::InsertContainer( Item it, CContainer* cc ) {

	if ( (this != cc) && (items.size() < MAXCONTAINERITEMS) && VolOk(it, cc) ) {
		Item titem = it;
		if ( items.size() < MAXITEMS ) { // es ist noch Platz frei
			CONTAINERMAP::iterator iterat;
			CONTAINERMAP::key_type count = 0;
			// freie Container-ID finden
			iterat = containers.find( 0 );
			while ( ( iterat != containers.end() ) && ( count < ( MAXITEMS - 1 ) ) ) {
				count = count + 1;
				iterat = containers.find( count );
			}
			// dem Container seine neue ID zuweisen
			titem.number = count;
			// den Container in die Itemliste einfügen
			items.push_back( titem );
			// den Inhalt des Containers in die Containermap mit der entsprechenden ID einfügen
			containers.insert( CONTAINERMAP::value_type( count, cc ) );
			return true;
		}
	}

	return false;

}


bool CContainer::changeQuality(TYPE_OF_ITEM_ID id, short int amount) {
	ITEMVECTOR::iterator theIterator = items.begin();
	short int tmpQuality;
	while ( theIterator < items.end() ) {
		if ( ContainerItems->find( theIterator->id ) ) {
			// Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) {
				// Inhalt des Containers gefunden
				// rekursiv verarbeiten
				if ( ( *iterat ).second->changeQuality( id, amount ) ) return true;
			}
			theIterator++;
		} else if  ( theIterator->id == id )     // Container als Items erstmal auslassen
		{
			tmpQuality = ((amount+theIterator->quality%100)<100) ? (amount + theIterator->quality) : (theIterator->quality-theIterator->quality%100 + 99);
			if ( tmpQuality%100 > 1) {
				theIterator->quality = tmpQuality;
				return true;
			} else {
				items.erase( theIterator );
				return true;
			}
		}
		else {
			theIterator++;
		}
	}
	return false;
}

bool CContainer::changeQualityAt(MAXCOUNTTYPE nr, short int amount) {
	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE count2 = 0;
	theIterator = items.begin();
	Item it;
	short int tmpQuality;

	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator < items.end() ) && ( count2 != nr ) ) {
		count2++;
		theIterator++;
	}
	if ( theIterator < items.end() ) {
		tmpQuality = ((amount+theIterator->quality%100)<100) ? (amount + theIterator->quality) : (theIterator->quality-theIterator->quality%100 + 99);
		if ( tmpQuality%100 > 1) {
			theIterator->quality = tmpQuality;
			return true;
		} else {
			//Falls Item ein Tasche den Inhalt löschen
			if ( ContainerItems->find( theIterator->id ) ) {
				CONTAINERMAP::iterator iterat = containers.find( (*theIterator).number );
				if ( iterat != containers.end() ) containers.erase( iterat );
			}
			items.erase( theIterator );
			return true;
		}
	}
	return false;
}


bool CContainer::TakeItemNr( MAXCOUNTTYPE nr, Item &it, CContainer* &cc, unsigned char count ) {

	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE count2 = 0;
	theIterator = items.begin();

	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator < items.end() ) && ( count2 != nr ) ) {
		count2++;
		theIterator++;
	}
#ifdef CContainer_DEBUG
	std::cout << "nr: " << nr << " count2: " << count2 << "\n";
#endif

	if ( theIterator < items.end() ) {
#ifdef CContainer_DEBUG
		std::cout << "das Item wurde gefunden,id: " << it.id << " number: " << it.number << "\n";
#endif
		it = *theIterator;
		ContainerStruct cont;
		if ( ContainerItems->find( it.id, cont ) ) {
			// das Item aus dem Vektor löschen
			items.erase( theIterator );
			CONTAINERMAP::iterator iterat = containers.find( it.number );
			if ( iterat != containers.end() ) {
#ifdef CContainer_DEBUG
				std::cout << "Inhalt des Containers gefunden\n";
#endif
				cc = ( *iterat ).second;
				// Inhalt löschen
				containers.erase( iterat );
			} else {
				cc = new CContainer(cont.ContainerVolume);
			}

			return true;

		} else {
			cc = NULL;
			//Itemstacking deaktivieren
			if ( isItemStackable(it) && count > 1 ) {
				if (theIterator->number > count) {
					theIterator->number-=count;
					it.number=count;
				} else {
					items.erase( theIterator );
				}
			} else {
				if ( theIterator->number > 1) {
					theIterator->number-=1;
					it.number=1;
				} else {
					items.erase( theIterator );
				}
			}
			//Itemstacking deaktivieren
		}

		return true;

	}

	// das Item wurde nicht gefunden
	else {
		it.id = 0;
		it.number = 0;
		it.wear = 0;
		cc = NULL;
		return false;
	}

}

luabind::object CContainer::getItemList()
{
    lua_State* _luaState = CWorld::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);
    int index = 1;
    MAXCOUNTTYPE pos = 0;
    ITEMVECTOR::iterator theIterator;
    for ( theIterator = items.begin(); theIterator != items.end(); ++theIterator ) 
    {

		ScriptItem item=*theIterator;
        item.type = ScriptItem::it_container;
        item.itempos = pos;
        item.inside = this;
        list[index] = item;
        index++;
        ++pos;
        if ( ContainerItems->find( theIterator->id ) ) 
        { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) 
            { 	// rekursiv verarbeiten
				iterat->second->increaseItemList(list, index);
			}
		}

	} 
    return list;
}

luabind::object CContainer::getItemList(TYPE_OF_ITEM_ID itemid)
{
    lua_State* _luaState = CWorld::get()->getCurrentScript()->getLuaState();
    luabind::object list = luabind::newtable(_luaState);
    int index = 1;
    MAXCOUNTTYPE pos = 0;
    ITEMVECTOR::iterator theIterator;
    for ( theIterator = items.begin(); theIterator != items.end(); ++theIterator ) 
    {

		if ( theIterator->id == itemid ) 
        {
            ScriptItem item=*theIterator;
            item.type = ScriptItem::it_container;
            item.itempos = pos;
            item.inside = this;
            list[index] = item;
            index++;
        }
        ++pos;
        if ( ContainerItems->find( theIterator->id ) ) 
        { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) 
            { 	// rekursiv verarbeiten
				iterat->second->increaseItemList( itemid , list, index);
			}
		}

	} 
    return list;
}

void CContainer::increaseItemList(TYPE_OF_ITEM_ID itemid, luabind::object &list, int &index)
{
	ITEMVECTOR::iterator theIterator;
    MAXCOUNTTYPE pos = 0;
	for ( theIterator = items.begin(); theIterator != items.end(); ++theIterator ) 
    {

		if ( theIterator->id == itemid ) 
        {
            ScriptItem item=*theIterator;
            item.type = ScriptItem::it_container;
            item.itempos = pos;
            item.inside = this;
            list[index] = item;
            index++;
        }
        ++pos;
        if ( ContainerItems->find( theIterator->id ) ) 
        { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) 
            { 	// rekursiv verarbeiten
				iterat->second->increaseItemList( itemid , list, index);
			}
		}

	}    
}

void CContainer::increaseItemList(luabind::object &list, int &index)
{
	ITEMVECTOR::iterator theIterator;
    MAXCOUNTTYPE pos = 0;
	for ( theIterator = items.begin(); theIterator != items.end(); ++theIterator ) 
    {

		ScriptItem item=*theIterator;
        item.type = ScriptItem::it_container;
        item.itempos = pos;
        item.inside = this;
        list[index] = item;
        index++;
        ++pos;
        if ( ContainerItems->find( theIterator->id ) ) 
        { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) 
            { 	// rekursiv verarbeiten
				iterat->second->increaseItemList(list, index);
			}
		}

	}    
}


bool CContainer::viewItemNr( MAXCOUNTTYPE nr, ScriptItem &it, CContainer* &cc ) {
	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE count = 0;
	theIterator = items.begin();
	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator < items.end() ) && ( count != nr ) ) {
		count++;
		theIterator++;
	}

	if ( theIterator < items.end() ) { // das Item wurde gefunden
		it = *theIterator;
        it.type = ScriptItem::it_container;
        it.itempos = nr;
        it.inside = this;
		ContainerStruct cont;
		if ( ContainerItems->find( it.id, cont ) ) {
			CONTAINERMAP::iterator iterat = containers.find( it.number );
			if ( iterat != containers.end() ) { // Inhalt des Containers gefunden
				cc = ( *iterat ).second;
			} else {
				cc = new CContainer(cont.ContainerVolume);
			}
		} else {
			cc = NULL;
		}
		return true;
	} else { // das Item wurde nicht gefunden
		it.id = 0;
		it.number = 0;
		it.wear = 0;
		cc = NULL;
		return false;
	}
}


int CContainer::increaseAtPos( unsigned char pos, int count ) {

	int temp = count;
#ifdef CContainer_DEBUG
	std::cout << "increaseAtPos " << ( short int ) pos << " " << count << "\n";
#endif
	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE tcount = 0;
	theIterator = items.begin();
	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator != items.end() ) && ( tcount != pos ) ) {
		tcount++;
		theIterator++;
	}

	if ( theIterator != items.end() ) { // das Item wurde gefunden
		if ( ContainerItems->find( ( *theIterator ).id ) ) {
			return count;     // container nicht verändern
		} else {
			temp = ( *theIterator ).number + count;
#ifdef CContainer_DEBUG
			std::cout << "temp " << temp << "\n";
#endif
			if ( temp > 255 ) {
				( *theIterator ).number = 255;
				temp = temp - 255;
			} else if ( temp <= 0 ) {
				temp = count + ( *theIterator ).number;
				items.erase( theIterator );
			} else {
				( *theIterator ).number = temp;
				temp = 0;
			}
		}
	}

	return temp;

}

bool CContainer::changeItem( ScriptItem it)
{
#ifdef CContainer_DEBUG
	std::cout << "swapAtPos " << ( short int ) pos << " " << newid << "\n";
#endif
	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE tcount = 0;
	theIterator = items.begin();
	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator != items.end() ) && ( tcount != it.itempos ) ) {
		tcount++;
		theIterator++;
	}

	if ( theIterator != items.end() ) { // das Item wurde gefunden
		if ( !ContainerItems->find( theIterator->id ) )
        {// container nicht verändern
			theIterator->id = it.id;
            theIterator->quality = it.quality;
            //theIterator->data = it.data;
            //theIterator->setData(it.data);
            theIterator->number = it.number;
            theIterator->data_map = it.data_map;
			//Wenn ein Qualitätswert angegeben ist neue Qualität setzen
			return true;
		}
	}

	return false;    
    
}

bool CContainer::swapAtPos( unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality ) {

#ifdef CContainer_DEBUG
	std::cout << "swapAtPos " << ( short int ) pos << " " << newid << "\n";
#endif
	ITEMVECTOR::iterator theIterator;
	MAXCOUNTTYPE tcount = 0;
	theIterator = items.begin();
	// das Item Nummer nr in dem Vektor finden
	while ( ( theIterator != items.end() ) && ( tcount != pos ) ) {
		tcount++;
		theIterator++;
	}

	if ( theIterator != items.end() ) { // das Item wurde gefunden
		if ( !ContainerItems->find( ( *theIterator ).id ) ) {// container nicht verändern
			( *theIterator ).id = newid;
			//Wenn ein Qualitätswert angegeben ist neue Qualität setzen
			if ( newQuality > 0 )  (*theIterator).quality = newQuality;
			return true;
		}
	}

	return false;

}

void CContainer::Save( std::ofstream* where ) {

	ITEMVECTOR::iterator theIterator;

	MAXCOUNTTYPE size = items.size();
	where->write( ( char* ) & size, sizeof( size ) );

	for ( theIterator = items.begin(); theIterator != items.end(); ++theIterator ) {
		where->write( ( char* ) &(*theIterator), sizeof( Item ) );

		// Item ist ein Container
		if ( ContainerItems->find( theIterator->id ) ) {
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			// Inhalt des Containers gefunden
			if ( iterat != containers.end() ) {
				// rekursiv Abspeichern
				( *iterat ).second->Save( where );
			}
			// leerer Container
			else {
				size = 0;
				where->write( ( char* ) & size, sizeof( size ) );
			}
		}
	}

}


void CContainer::Load( std::istream* where ) {

	if ( !containers.empty() ) {
		CONTAINERMAP::iterator theIterator;
		for ( theIterator = containers.begin(); theIterator != containers.end(); ++theIterator ) {
			delete ( *theIterator ).second;
			( *theIterator ).second = NULL;
		}
	}

	items.clear();
	containers.clear();

	MAXCOUNTTYPE size;
	where->read( ( char* ) & size, sizeof( size ) );

	CContainer* tempc;
	ContainerStruct cont;

	Item tempi;
	for ( int i = 0; i < size; ++i ) {
		where->read( ( char* ) & tempi, sizeof( tempi ) );
		// das Item ist ein Container
		if ( ContainerItems->find( tempi.id, cont ) ) {
			tempc = new CContainer(cont.ContainerVolume);
			tempc->Load( where );
			InsertContainer( tempi, tempc );
		} else {
			InsertItem( tempi, false );
		}
	}
}


int CContainer::countItem( TYPE_OF_ITEM_ID itemid ) {
	int temp = 0;
	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) {
		if ( theIterator->id == itemid && theIterator->quality >= 100 ) {
			temp = temp + theIterator->number;
		}

		if ( ContainerItems->find( theIterator->id ) ) { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) { // Inhalt des Containers gefunden
				// rekursiv verarbeiten
				temp = temp + ( *iterat ).second->countItem( itemid );
			}
		}
	}
	return temp;
}

int CContainer::countItem( TYPE_OF_ITEM_ID itemid, uint32_t data ) 
{
	int temp = 0;
	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) 
    {
		if ( theIterator->id == itemid && theIterator->data == data && theIterator->quality >= 100 ) 
        {
			temp = temp + theIterator->number;
		}

		if ( ContainerItems->find( theIterator->id ) ) 
        { // Item ist ein Container
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) 
            { // Inhalt des Containers gefunden, rekursiv verarbeiten
				temp = temp + ( *iterat ).second->countItem( itemid , data);
			}
		}
	}
	return temp;
}

uint16_t CContainer::Volume(int rekt) {
#ifdef CContainer_DEBUG
	std::cout << "in CContainer::Volume rekt: "<<rekt<< std::endl;
#endif
    int temprekt = rekt + 1;
#ifdef CContainer_DEBUG
	std::cout << "temprekt: "<<temprekt<< std::endl;
#endif

	if ( rekt > MAXIMALEREKURSIONSTIEFE ) {
		throw CRekursionException();
	}

	uint32_t temp = 0;
	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) 
    {
		if ( !CommonItems->find( theIterator->id, tempCommon ) ) 
        {
			tempCommon.Volume = 0;
		}
		if ( ContainerItems->find( theIterator->id ) )
		{
		    CONTAINERMAP::iterator iterat = containers.find(theIterator->number);
		    if ( iterat != containers.end() )
		    {
#ifdef CContainer_DEBUG
	std::cout << "found another container inside the first one" << std::endl;
#endif		    //rekursiv verarbeiten
		        temp += (*iterat).second->Volume(temprekt);
            }
            temp += tempCommon.Volume;
        }
        else 
            temp += ( tempCommon.Volume * theIterator->number );
	}

	if ( temp > 65000 ) {
		return 65000;
	} else {
		return temp;
	}
}


int CContainer::weight(int rekt) {
	int temprekt = rekt + 1;

	if ( rekt > MAXIMALEREKURSIONSTIEFE ) {
		throw CRekursionException();
	}

	uint32_t temp = 0;
	ITEMVECTOR::iterator theIterator;

	for ( theIterator = items.begin(); theIterator < items.end(); ++theIterator ) {
		if ( !CommonItems->find( theIterator->id, tempCommon ) ) {
			tempCommon.Weight = 0;
		}

		if ( ContainerItems->find( theIterator->id ) ) { // Item ist ein Container -> number ist nur die ID für den Containerinhalt
			CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
			if ( iterat != containers.end() ) { // Inhalt des Containers gefunden
				// rekursiv verarbeiten
				temp += ( *iterat ).second->weight(temprekt);
			}
			temp += tempCommon.Weight;
		} else { // kein Container -> Anzahl mit Gewicht multiplizieren
			temp += ( tempCommon.Weight * theIterator->number );
		}
	}

	if ( temp > 30000 ) {
		return 30000;
	} else {
		return temp;
	}
}


int CContainer::_eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data, bool useData ) {

#ifdef CContainer_DEBUG
    std::cout << "in CContainer::eraseItem" << std::endl;
#endif
    int temp = count;

    ITEMVECTOR::iterator theIterator = items.begin();
    while ( theIterator < items.end() ) {
        if ( ( ContainerItems->find( theIterator->id ) ) && ( temp > 0 ) ) { // Item ist ein Container
            CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
            if ( iterat != containers.end() ) { // Inhalt des Containers gefunden
                // rekursiv verarbeiten
                temp = ( *iterat ).second->_eraseItem( itemid, temp, data, useData );
            }
            theIterator++;
        } else if ( ( theIterator->id == itemid && (!useData || theIterator->data == data) && theIterator->quality >=100 ) && ( temp > 0 ) )     // Container als Items erstmal auslassen
        {
#ifdef CContainer_DEBUG
            std::cout << "Item mit Anzahl " << ( unsigned short ) theIterator->number << " gefunden!\n";
#endif
            if ( temp >= theIterator->number ) {
                temp = temp - theIterator->number;
                theIterator = items.erase( theIterator );
#ifdef CContainer_DEBUG
                std::cout << "Anzahl zu gering, noch " << temp << "zu loeschen!\n";
#endif

            } else {
                theIterator->number = theIterator->number - temp;
                temp = 0;
#ifdef CContainer_DEBUG
                std::cout << "Anzahl ausreichend\n";
#endif
                theIterator++;
            }
        }
        else {
            theIterator++;
        }
    }
    std::cout << "am Ende von CContainer::eraseItem" << std::endl;
    return temp;
}


int CContainer::eraseItem( TYPE_OF_ITEM_ID itemid, int count ) {
    return _eraseItem( itemid, count, 0, false );
}


int CContainer::eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data ) {
    return _eraseItem( itemid, count, data, true );
}


void CContainer::doAge( ITEM_FUNCT funct, bool inventory )
{
	if ( !items.empty() ) 
    {
		ITEMVECTOR::iterator theIterator = items.begin();
		while ( theIterator < items.end() ) 
        {
            if ( !CommonItems->find( theIterator->id, tempCommon ) )
            {
                tempCommon.ObjectAfterRot = theIterator->id;
                tempCommon.rotsInInventory = false;
            }
            if ( !(inventory) || (inventory && tempCommon.rotsInInventory) )
            {
                if ( !funct( &(*theIterator) ) ) 
                {
                    tempCommon.ObjectAfterRot = theIterator->id;
                    if ( theIterator->id != tempCommon.ObjectAfterRot ) 
                    {
                        #ifdef CContainer_DEBUG
                        std::cout << "CContainer:Ein Item wird umgewandelt von: " << theIterator->id << "  nach: " << tempCommon.ObjectAfterRot << "!\n";
                        #endif
                        theIterator->id = tempCommon.ObjectAfterRot;
                        if ( CommonItems->find( tempCommon.ObjectAfterRot, tempCommon ) ) 
                        {
                            theIterator->wear = tempCommon.AgeingSpeed;
                        }
                        theIterator++;
                    } 
                    else 
                    {
                        #ifdef CContainer_DEBUG
                        std::cout << "CContainer:Ein Item wird gelöscht,ID:" << theIterator->id << "!\n";
                        #endif
                        if ( ContainerItems->find( theIterator->id ) ) 
                        {
                            CONTAINERMAP::iterator iterat = containers.find( theIterator->number );
                            if ( iterat != containers.end() ) 
                            {
                                #ifdef CContainer_DEBUG
                                std::cout << "Inhalt des Containers gefunden\n";
                                #endif
                                // Inhalt löschen
                                containers.erase( iterat );
                            }
                        }
                        // das Item aus dem Vektor löschen
                        theIterator = items.erase( theIterator );
                    }
                }
                else 
                {
                    theIterator++;
                }
            }
            else
            {
                theIterator++;
            }
        }
	}

	CONTAINERMAP::iterator iterat;

	if ( !containers.empty() ) {
		for ( iterat = containers.begin(); iterat != containers.end(); ++iterat ) {
			( *iterat ).second->doAge( funct, inventory );
		}
	}

}

bool CContainer::isItemStackable(Item item) {
	CommonStruct com;
	if ( CommonItems->find(item.id, com) ) {
		return com.isStackable;
	} else {
		std::cerr<<"Item mit folgender id konnte nicht gefunden werden in Funktion isItemStackable(Item item): "<<item.id<<" !" <<std::endl;
		return false;
	}
	return false;
}

bool CContainer::VolOk(Item item) {
#ifdef CContainer_DEBUG
	std::cout << "in CContainer::VolOk(Item item)" << std::endl;
#endif
	CommonStruct com;
	if ( CommonItems->find(item.id, com) )
		//true zurück liefern wenn das max Volumen unterschritten ist bzw. max Volumen == 0 (unendlich volumen bei Depot)
		try
		{
		     return ( ( max_Volume == 0 ) || ( (com.Volume * item.number + Volume(0)) <= max_Volume) ) ;
        }
        catch ( CRekursionException e)
        {
            std::cerr<<"VolOk(Item item) maximale Rekursionstiefe: "<<MAXIMALEREKURSIONSTIEFE<<" wurde erreicht!"<<std::endl;
            return false;
        }
	else
		return false;
	return false;
}

bool CContainer::VolOk(Item item, CContainer * cont)
{
#ifdef CContainer_DEBUG
	std::cout << "in CContainer::VolOk(Item item, Container * cont)" << std::endl;
#endif
	CommonStruct com;
	if ( CommonItems->find(item.id, com) )
	{
#ifdef CContainer_DEBUG
	std::cout << "Volume of the Container ="<<com.Volume<< std::endl;
	std::cout << "Volume of the items inside the container ="<<Volume(0)<<std::endl;
	std::cout << "Volume of the new inserterd container = "<<cont->Volume(0)<<std::endl;
#endif
		//true zurück liefern wenn das max Volumen unterschritten ist bzw. max Volumen == 0 (unendlich volumen bei Depot)
		try
		{
            return ( ( max_Volume == 0 ) || ( (com.Volume + Volume(0) + cont->Volume(0) ) <= max_Volume) ) ;
        }
        catch ( CRekursionException e)
        {
            std::cerr<<"VolOk(Item item) maximale Rekursionstiefe: "<<MAXIMALEREKURSIONSTIEFE<<" wurde erreicht!"<<std::endl;
            return false;
        }      
    }
	else
		return false;
	return false;
}
