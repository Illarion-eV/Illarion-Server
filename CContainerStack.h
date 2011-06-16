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


#ifndef CCONTAINERSTACK_HH
#define CCONTAINERSTACK_HH

#include <vector>
#include <iterator>

#include "CContainer.h"


//! eine Darstellung offener Container eines Spielers
class CContainerStack {
	public:
		//! Konstruktor
		CContainerStack();

		//! Copy-Konstruktor
		CContainerStack( const CContainerStack& source );

		//! Destruktor
		~CContainerStack();

		//! Zuweisungsoperator
		CContainerStack& operator =( const CContainerStack& source );

		//! den Start - CContainer zuweisen, in dem man arbeiten kann
		// \param cc der Zeiger auf den Startcontainer
		// \param carry gibt an, ob der Spieler diesen Container trägt
		void startContainer( CContainer* cc, bool carry );

		//! entfernt alle Zeiger auf CContainer aus der Datenstruktur
		void clear();

		//! einen neuen Zeiger auf CContainer hinzufügen
		//
		// \param cc der Zeiger auf den CContainer der geöffnet werden soll
		void openContainer( CContainer* cc );

		//! entfernt den zuletzt hinzugefügten Zeiger auf CContainer
		// \return true, falls noch Zeiger auf CContainer vorhanden sind, false sonst
		bool closeContainer();

		//! prüft, ob ein Spieler die CContainer trägt
		// \return true, falls ein Spieler die CContainer trägt, false sonst
		bool inInventory();

		//! prüft, ob der übergebene Zeiger auf CContainer der zuletzt hinzugefügte ist
		// \param cc Zeiger auf CContainer der überprüft werden soll
		// \return true, falls cc zuletzt hinzugefügt wurde, false sonst
		bool isOnTop( CContainer* cc );

		//! liefert den zuletzt hinzugefügten Zeiger auf CContainer zurück
		// \param cc Zeiger auf CContainer der überprüft werden soll
		// \return NULL, falls kein Eintrag vorhanden
		CContainer* top();

		//! prüft, ob der übergebene Zeiger in dem Stack ist
		// \param cc Zeiger auf CContainer der überprüft werden soll
		// \return true, falls cc in dem Stack ist, false sonst
		bool contains( CContainer* cc );

		//! gibt an, ob ein Spieler die Container trägt
		bool inventory;

	private:
		//! definiert eine Template-Klasse "std::vector für Zeiger auf CContainer"
		typedef std::vector < CContainer* > CONTAINERVECTOR;

		//! ein std::vector von Zeigern auf CContainer
		CONTAINERVECTOR opencontainers;
};

#endif
