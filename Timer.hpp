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


#ifndef TIMER_HH
#define TIMER_HH

#include <ctime>

//! Timer für Sekundenabstände
class Timer {

	public:

		//! Konstruktor
		// \param timegap Anzahl der Sekunden zwischen zwei Timer-Ereignissen
		Timer( unsigned long int timegap );

		//! Destruktor
		~Timer();

		//! prüft, ob schon genügend Zeit seit dem letzten Timer-Ereignis
		// (bzw. dem Konstruktor) vergangen ist
		// \return true, falls das nächste Ereignis anliegt, false sonst
		bool next();

	private:

		//! Zeitpunkt der letzten Alterung (Anzahl der Sekunden seit 1.1.1970)
		time_t last;     //(32 Bit Integer)

		//! Anzahl der Sekunden zwischen den Schritten
		long gap;

		//! Sekunden seit dem letzten Ereignis
		long realgap;
};
#endif

