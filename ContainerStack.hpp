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


#ifndef CONTAINERSTACK_HH
#define CONTAINERSTACK_HH

#include <vector>
#include <iterator>

#include "Container.hpp"


//! eine Darstellung offener Container eines Spielers
class ContainerStack {
	public:
		//! Konstruktor
		ContainerStack();

		//! Copy-Konstruktor
		ContainerStack( const ContainerStack& source );

		//! Destruktor
		~ContainerStack();

		//! Zuweisungsoperator
		ContainerStack& operator =( const ContainerStack& source );

		//! den Start - Container zuweisen, in dem man arbeiten kann
		// \param cc der Zeiger auf den Startcontainer
		// \param carry gibt an, ob der Spieler diesen Container trägt
		void startContainer( Container* cc, bool carry );

		//! entfernt alle Zeiger auf Container aus der Datenstruktur
		void clear();

		//! einen neuen Zeiger auf Container hinzufügen
		//
		// \param cc der Zeiger auf den Container der geöffnet werden soll
		void openContainer( Container* cc );

		//! entfernt den zuletzt hinzugefügten Zeiger auf Container
		// \return true, falls noch Zeiger auf Container vorhanden sind, false sonst
		bool closeContainer();

		//! prüft, ob ein Spieler die Container trägt
		// \return true, falls ein Spieler die Container trägt, false sonst
		bool inInventory();

		//! prüft, ob der übergebene Zeiger auf Container der zuletzt hinzugefügte ist
		// \param cc Zeiger auf Container der überprüft werden soll
		// \return true, falls cc zuletzt hinzugefügt wurde, false sonst
		bool isOnTop( Container* cc );

		//! liefert den zuletzt hinzugefügten Zeiger auf Container zurück
		// \param cc Zeiger auf Container der überprüft werden soll
		// \return NULL, falls kein Eintrag vorhanden
		Container* top();

		//! prüft, ob der übergebene Zeiger in dem Stack ist
		// \param cc Zeiger auf Container der überprüft werden soll
		// \return true, falls cc in dem Stack ist, false sonst
		bool contains( Container* cc );

		//! gibt an, ob ein Spieler die Container trägt
		bool inventory;

	private:
		//! definiert eine Template-Klasse "std::vector für Zeiger auf Container"
		typedef std::vector < Container* > ONTAINERVECTOR;

		//! ein std::vector von Zeigern auf Container
		ONTAINERVECTOR opencontainers;
};

#endif
