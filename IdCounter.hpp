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


#ifndef IdCounter_HH
#define IdCounter_HH

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
//#define IdCounter_DEBUG

#include <string>
#include <fstream>
#include <iostream>

#include "types.hpp"

//! Ein Zähler für bereits vergebene ID's
// Der Zählerstand wird in eine Datei gespeichert und bei
// Initialisierung wieder geladen.
class IdCounter {
public:
    //! Konstruktor
    // \param counterfile die Datei welche für die Speicherung der
    // zuletzt vergebenen ID genutzt wird
    IdCounter(std::string counterfile);

    // create a new counter with the given starting ID
    IdCounter(const std::string &counterfile, const TYPE_OF_CHARACTER_ID &startid);
    //! Destruktor
    ~IdCounter();

    //! liefert die nächste freie ID zurück und hält diese als bereits verwendet fest
    // \return die nächste freie ID
    TYPE_OF_CHARACTER_ID nextFreeId();

    //! setzt den Counter wieder auf 0 zurück
    // \return true falls Counter erfolgreich zurückgesetzt, false sonst
    bool reset();

    //! setzt den Counter
    // \param lastid die zuletzt vergebene ID
    // \return true falls Counter erfolgreich gesetzt, false sonst
    bool set(TYPE_OF_CHARACTER_ID lastid);

private:
    //! der Name der Datei in dem die letzte bereits vergebene ID gespeichert ist
    std::string counterfile;

    //! die letzte bereits vergebene ID
    TYPE_OF_CHARACTER_ID counter;
};

#endif
