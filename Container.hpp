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


#ifndef CONTAINER_HH
#define CONTAINER_HH

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
//#define Container_DEBUG

#define MAXIMALEREKURSIONSTIEFE 100

#include "TableStructs.hpp"

#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sys/types.h>

class RekursionException: public std::exception {};

#include "Item.hpp"
#include "constants.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

// forward declarations
class ContainerObjectTable;
class CommonObjectTable;


//! eine Tabelle für Behälter - Item Daten
extern ContainerObjectTable* ContainerItems;

//! eine Tabelle mit den allgemeinen Attributen der Item
extern CommonObjectTable* CommonItems;

//! wird von verschiedenen Funktionen als Zwischenvariable genutzt
extern CommonStruct tempCommon;

//! ein Container
class Container {
	public:
		//! die Gegenstände in dem Container
		ITEMVECTOR items;

		//! definiert eine Template-Klasse "map mit key unsigned char für Zeiger auf Container"
		typedef std::map < MAXCOUNTTYPE , Container*, std::less < MAXCOUNTTYPE > > ONTAINERMAP;

		//! alle Gegenstände der Item in items, die selbst Container sind
		ONTAINERMAP containers;

		//! Konstruktor
		Container(uint16_t max_cont_Volume);

		//! Copy-Konstruktor
		Container( const Container& source );

		//! Destruktor
		~Container();

		//! Zuweisungsoperator
		Container& operator =( const Container& source );

		//! nimmt das Item von der Position nr
		// \param nr die Position des Item
		// \param it Rückgabe des Item
		// \param cc Rückgabe des Inhalts des Item, falls es ein Container ist
		// \param count Anzahl
		// \return true, falls ein Item an der Position nr gefunden
		// wurde, false sonst
		bool TakeItemNr( MAXCOUNTTYPE nr, Item &it, Container* &cc, unsigned char count );

		//! liefert das Item an der Position nr
		// \param nr die Position des Items
		// \param it Rückgabe des ScriptItems
		// \param cc Rückgabe des Inhalts des Items, falls es ein Container ist
		// \return true, falls ein Item an der Position nr gefunden
		// wurde, false sonst
		bool viewItemNr( MAXCOUNTTYPE nr, ScriptItem &it, Container* &cc );

		//Verringert die Qualität eines Items an einer bestimmten stelle
		//\param nr, de stelle an der die Qualität verringert werden soll
		//\param amount, um wieviel die qualität verringert werden soll
		//\return true wenn das ändern geklappt hat
		bool changeQualityAt( MAXCOUNTTYPE nr, short int amount);

		//Verringert die Qualität eines Items mit einer bestimmten Id
		//\param id, die Id des Items dessen Qualität verringert werden soll
		//\param amount, um wieviel die qualität verringert werden soll
		//\return true wenn das ändern geklappt hat
		bool changeQuality(TYPE_OF_ITEM_ID id, short int amount);

		//! fügt ein Item das ein Container ist in den Container ein
		// \param it das Item zum Einfügen (sollte ID eines Container-Item haben)
		// \param cc der Inhalt des Item
		// \return true, falls erfolgreich, false sonst
		bool InsertContainer( Item it, Container* cc );
        

		//! fügt ein Item in den Container ein
		// \param it das Item zum Einfügen
		// \param merge falls true wird die Anzahl von vorhandenen Item mit dem gleichen Typ wie it
		// erhöht, anstatt ein neues Item einzufügen
		// \return true, falls erfolgreich, false sonst
		bool InsertItem( Item it, bool merge );

		//! fügt ein Item in den Container ein
		// \param it das Item zum Einfügen
		// \param pos hat das einzufügende Item die gleiche Id wie das Item an der Position pos, dann wird nur die Anzahl addiert und ggf. der Rest als neues Item eingefügt
		// \return true, falls erfolgreich, false sonst
		bool InsertItem( Item it, unsigned char pos);
        
        bool InsertItemOnLoad( Item it );

		//! fügt ein Item in den Container ein
		// \param it das Item zum Einfügen
		// \return true, falls erfolgreich, false sonst
		bool InsertItem( Item it );

		//! speichert den Inhalt des Containers rekursiv in einen Outputstream
		// \param where der Outputstream in den gespeichert werden soll
		void Save( std::ofstream* where );

		//! lädt den Inhalt des Containers rekursiv aus einem Inputstream
		// \param where der Inputstream aus dem geladen wird
		void Load( std::istream* where );

		//! altert die Item in dem Container
		// \param funct
		// \see Item.hpp
		void doAge( ITEM_FUNCT funct, bool inventory = false );

		//! zählt die Item mit der Id itemid im Container
		// \param itemid die Id nach der gesucht wird
		// \return die Anzahl der gefunden Item mit der Id itemid
		int countItem( TYPE_OF_ITEM_ID itemid );
        int countItem( TYPE_OF_ITEM_ID itemid, uint32_t data);
        
        /**
        *adds items of a specific id to a luabindlist
        */
        void increaseItemList(TYPE_OF_ITEM_ID itemid, luabind::object &list,int &index);
        void increaseItemList(luabind::object &list,int &index);
        
        /**
        *returns a list of items to lua
        */
        luabind::object getItemList(TYPE_OF_ITEM_ID itemid);
        luabind::object getItemList();
        
		//! löscht count Item mit der Id itemid im Container
		// \param itemid die Id nach der gesucht wird
		// \param count Änderung
		// \return der Rest von count der nicht verarbeitet werden konnte
		int eraseItem( TYPE_OF_ITEM_ID itemid, int count );
        int eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data );

		//! verändert die Anzahl des Item an Position pos um count
		// \param pos die Position des Item im Container
		// \param count Änderung
		// \return der Rest von count der nicht verarbeitet werden konnte
		int increaseAtPos( unsigned char pos, int count );

		//! tauscht die Id des Item an der Position pos gegen newid aus
		// \param pos die Position des Item im Container
		// \param die neue Id des Item
		// \return true falls erfolgreich, false sonst
		bool swapAtPos( unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0 );
        
        /**
        *changes a item at a specific pos
        */
        bool changeItem( ScriptItem It);

		//! liefert das Gesamtgewicht des Containers
		// \return das Gesamtgewicht des Containers
		int weight(int rekt);

		//! liefert das Volumen des inhaltes des Containers
		// \return das Volumen
		uint16_t Volume(int rekt);

        //! called by both eraseItem variants to prevent multiple instances of the same code
        int _eraseItem( TYPE_OF_ITEM_ID itemid, int count, uint32_t data, bool useData );


protected:

		//! Prüft ob ein Item Stabelbar ist
		//\item das Item welches geprüft werden soll
		//\return true wenn das Item Stapelbar ist
		bool isItemStackable(Item item);

		//!Prüft ob das Max Volumen eines Containers überschritten ist wenn item hinzu gefügt wird.
		bool VolOk(Item item);

		//!Prüft ob das MaxVolumen eines Containers überschritten ist wenn der neue container eingefügt wird.
		bool VolOk(Item item, Container * cont);

		uint16_t max_Volume;
};

#endif

