//  illarionserver - server for the game Illarion
//  Copyright 2011 Illarion e.V.
//
//  This file is part of illarionserver.
//
//  illarionserver is free software: you can redistribute it and/or modify
//  it under the terms of the GNU Affero General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  illarionserver is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Affero General Public License for more details.
//
//  You should have received a copy of the GNU Affero General Public License
//  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.


#ifndef TIMER_HH
#define TIMER_HH

#include <ctime>

//! Timer f�r Sekundenabst�nde
class Timer {

public:

    //! Konstruktor
    // \param timegap Anzahl der Sekunden zwischen zwei Timer-Ereignissen
    explicit Timer(unsigned long int timegap);

    //! Destruktor
    ~Timer();

    //! pr�ft, ob schon gen�gend Zeit seit dem letzten Timer-Ereignis
    // (bzw. dem Konstruktor) vergangen ist
    // \return true, falls das n�chste Ereignis anliegt, false sonst
    auto next() -> bool;

private:

    //! Zeitpunkt der letzten Alterung (Anzahl der Sekunden seit 1.1.1970)
    time_t last;     //(32 Bit Integer)

    //! Anzahl der Sekunden zwischen den Schritten
    long gap;
};
#endif

