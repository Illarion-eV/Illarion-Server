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


#ifndef _WORLDMAP_HPP_
#define _WORLDMAP_HPP_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "globals.hpp"

class Map;

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/* #define WorldMap_DEBUG */

//! eine std::vector-Klasse für Map-Objekte.
// Die Klasse erweitert die Funktionalität von std::vector um
// Map - spezifische Suchfunktionen
class WorldMap {
public:
    typedef boost::shared_ptr<Map> map_t;
    typedef std::vector<map_t> map_vector_t;

    WorldMap();

    void clear();

    bool findAllMapsInRangeOf(char rnorth, char rsouth, char reast, char rwest, position pos, map_vector_t &ret) const;
    bool mapInRangeOf(short int upperleft_X, short int upperleft_Y, unsigned short int sizex, unsigned short int sizey, short int z) const;
    bool findMapForPos(const position &pos, map_t &map) const;
    bool findMapForPos(short int x, short int y, short int z, map_t &map) const;
    bool findAllMapsWithXInRangeOf(short int start, short int end, map_vector_t &ret) const;

    bool InsertMap(map_t newMap);

    void ageContainers();

    bool exportTo(const std::string &exportDir) const;
    void saveToDisk(const std::string &prefix) const;

    short int getLowX() const {
        return lowX;
    };
    short int getHighX() const {
        return highX;
    };

private:
    map_vector_t maps;
    boost::unordered_map<position, map_t> world_map;
    short int lowX;
    short int highX;
};
#endif
