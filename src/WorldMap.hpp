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


#ifndef _WORLDMAP_HPP_
#define _WORLDMAP_HPP_

#include <memory>
#include <vector>
#include <unordered_map>
#include "globals.hpp"

class Map;

//falls nicht auskommentiert, werden mehr Bildschirmausgaben gemacht:
/* #define WorldMap_DEBUG */

//! eine std::vector-Klasse für Map-Objekte.
// Die Klasse erweitert die Funktionalität von std::vector um
// Map - spezifische Suchfunktionen
class WorldMap {
public:
    typedef std::shared_ptr<Map> map_t;
    typedef std::vector<map_t> map_vector_t;

    void clear();

    bool findAllMapsInRangeOf(char rnorth, char rsouth, char reast, char rwest, position pos, map_vector_t &ret) const;
    bool mapInRangeOf(const position &upperleft, unsigned short int dx, unsigned short int dy) const;
    bool findMapForPos(const position &pos, map_t &map) const;

    bool InsertMap(map_t newMap);

    void age();

    bool exportTo(const std::string &exportDir) const;
    void saveToDisk(const std::string &prefix) const;

private:
    map_vector_t maps;
    std::unordered_map<position, map_t> world_map;
};
#endif
