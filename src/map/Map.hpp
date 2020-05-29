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
    auto operator=(const Map &) -> Map & = delete;
    Map(Map &&) = default;
    auto operator=(Map &&) -> Map & = default;

    auto import(const std::string &importDir, const std::string &mapName) -> bool;
    auto load(const std::string &name) -> bool;
    auto save(const std::string &name) const -> bool;

    auto at(int16_t x, int16_t y) -> Field &;
    auto at(int16_t x, int16_t y) const -> const Field &;
    auto at(const MapPosition &) -> Field &;
    auto at(const MapPosition &) const -> const Field &;

    void age();

    auto getMinX() const -> int16_t;
    auto getMinY() const -> int16_t;
    auto getMaxX() const -> int16_t;
    auto getMaxY() const -> int16_t;
    auto getLevel() const -> int16_t;
    auto getWidth() const -> uint16_t;
    auto getHeight() const -> uint16_t;
    auto getName() const -> const std::string &;

    auto intersects(const Map &map) const -> bool;

private:
    auto importFields(const std::string &importDir, const std::string &mapName) -> bool;
    auto importItems(const std::string &importDir, const std::string &mapName) -> bool;
    auto importWarps(const std::string &importDir, const std::string &mapName) -> bool;
    static void unescape(std::string &input);

    inline auto convertWorldXToMap(int16_t x) const -> uint16_t;
    inline auto convertWorldYToMap(int16_t y) const -> uint16_t;
};

}

#endif
