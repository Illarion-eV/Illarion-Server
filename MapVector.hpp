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


#ifndef MAPVECTOR_HH
#define MAPVECTOR_HH

#include <vector>
#include "globals.hpp"

class Map;

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/* #define MapVector_DEBUG */

//! eine std::vector-Klasse für Map-Objekte.
// Die Klasse erweitert die Funktionalität von std::vector um
// Map - spezifische Suchfunktionen
class MapVector : private std::vector < Map * > {
public:
    using std::vector < Map * >::iterator;
    using std::vector < Map * >::begin;
    using std::vector < Map * >::end;
    using std::vector < Map * >::size;

    MapVector();

    void clear();

    //! sucht in dem std::vector nach Map mit Koordinaten in der Nähe von pos
    // \param rnorth maximaler Abstand der Map nach Norden
    // \param rsouth maximaler Abstand der Map nach Süden
    // \param reast maximaler Abstand der Map nach Osten
    // \param rwest maximaler Abstand der Map nach Westen
    // \param pos der Mittelpunkt des Sichtfensters
    // \param ret der Vektor mit den gefundenen Karten
    // \return true, falls mindestens eine Karte gunden wurde, false sonst
    bool findAllMapsInRangeOf(char rnorth, char rsouth, char reast, char rwest, position pos, std::vector < Map * > &ret);

    //! prüft, ob sich eine Map im angegebenen Bereich befindet
    // \param upperleft_X obere linke X-Koordinate
    // \param upperleft_Y obere linke Y-Koordinate
    // \param sizex Breite des Bereiches
    // \param sizey Höhe des Bereiches
    // \param z Z-Koordinate der Ebene
    // \return true falls siche eine Map in dem Bereich befindet
    bool mapInRangeOf(short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z);

    //! sucht in dem std::vector nach der Map auf der pos liegt
    // \param pos die gesuchte Koordinate
    // \param map die gesucht Map, falls sie gefunden wurde
    // \return false falls keine passende Map gefunden wurde, sonst true
    bool findMapForPos(position pos, Map* &map);

    //! sucht in dem std::vector nach der Map auf der (x,y,z) liegt
    // \param x die gesuchte X-Koordinate
    // \param y die gesuchte Y-Koordinate
    // \param z die gesuchte Z-Koordinate
    // \param map die gesucht Map, falls sie gefunden wurde
    // \return false falls keine passende Map gefunden wurde, sonst true
    bool findMapForPos(short int x, short int y, short int z, Map* &map);

    //! sucht in dem std::vector nach allen Map die die X-Koordinaten von start bis end berühren
    // \param start kleinste X-Koordinate
    // \param end größte X-Koordinate
    // \param ret ein std::vector mit Zeigern auf alle gefundenen Map
    // \return true, falls mindestens eine Map gefunden wurde, false sonst
    bool findAllMapsWithXInRangeOf(short int start, short int end, std::vector < Map * > &ret);

    //! sucht die niedrigste Karte über einer Position
    // \param pos die Position
    // \param lowmap die gefundene Karte
    // \return true falls eine Karte gefunden wurde, false sonst
    bool findLowestMapOverCharacter(position pos, Map* &lowmap);

    //! fügt eine Map in den std::vector ein und aktualisiert low_X und high_X
    // \param newMap die einzufügende Map
    // \return true falls die Map eingefügt werden konnte, false sonst
    bool InsertMap(Map *newMap);

    short int getLowX() {
        return lowX;
    };
    short int getHighX() {
        return highX;
    };

private:
    //! die kleinste X-Koordinate aller Map im std::vector
    short int lowX;

    //! die großte X-Koordinate aller Map im std::vector
    short int highX;
};
#endif
