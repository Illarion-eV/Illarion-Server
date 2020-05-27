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


#ifndef _MAP_HPP_
#define _MAP_HPP_

#include <string>
#include <unordered_map>
#include "globals.hpp"
#include "map/Field.hpp"
#include "Container.hpp"

namespace map {

class Map {
    position origin;
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<Field>> fields;
    std::string name;

public:
    Map(std::string name, position origin, uint16_t width, uint16_t height);
    Map(std::string name, position origin, uint16_t width, uint16_t height,
        uint16_t tile);
    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;
    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    bool import(const std::string &importDir, const std::string &mapName);
    bool load(const std::string &name);
    bool save(const std::string &name) const;

    Field &at(int16_t x, int16_t y);
    const Field &at(int16_t x, int16_t y) const;
    Field &at(const MapPosition &);
    const Field &at(const MapPosition &) const;

    void age();

    int16_t getMinX() const;
    int16_t getMinY() const;
    int16_t getMaxX() const;
    int16_t getMaxY() const;
    int16_t getLevel() const;
    uint16_t getWidth() const;
    uint16_t getHeight() const;
    const std::string &getName() const;

    bool intersects(const Map &map) const;

private:
    bool importFields(const std::string &importDir, const std::string &mapName);
    bool importItems(const std::string &importDir, const std::string &mapName);
    bool importWarps(const std::string &importDir, const std::string &mapName);
    static void unescape(std::string &input);

    inline uint16_t convertWorldXToMap(int16_t x) const;
    inline uint16_t convertWorldYToMap(int16_t y) const;
};

}

#endif
