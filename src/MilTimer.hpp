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

#ifndef MILTIMER_HH
#define MILTIMER_HH

#include <sys/timeb.h>

//! ein Timer mit Millisekunden-Aufl�sung
class MilTimer {
public:
    //! Konstruktor
    // \param milsec Anzahl der Millisekunden zwischen zwei Schritten
    explicit MilTimer(long milsec);

    //! dient zur Abfrage, ob die Zeit schon abgelaufen ist
    // \return true falls die im Konstruktor angegebene Zeit seit
    // dem letzten erfolgreichen Aufruf abgelaufen ist, sonst false
    auto Next() -> bool;

private:
    //! letzte Alterung
    timeb last{};

    //! die geforderte Zeitspanne in Millisekunden
    long gap;
};

#endif
