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

#include <exception>
#include <string>
#include <unordered_map>
#include "globals.hpp"
#include "Field.hpp"
#include "Container.hpp"

class Map {
    position origin;
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<Field>> fields;

public:
    std::unordered_map<MapPosition, Container::CONTAINERMAP> containers;

    Map(position origin, uint16_t width, uint16_t height);
    Map(const Map &) = delete;
    Map &operator=(const Map &) = delete;
    Map(Map &&) = default;
    Map &operator=(Map &&) = default;

    bool Load(const std::string &name);
    bool Save(const std::string &name) const;

    //Field &at(int16_t x, int16_t y) const;
    //Field &near(int16_t x, int16_t y) const;
    bool GetPToCFieldAt(Field *&fip, int16_t x, int16_t y);
    bool GetCFieldAt(Field &fi, int16_t x, int16_t y);
    bool findEmptyCFieldNear(Field *&cf, int16_t &x, int16_t &y);

    void age();

    bool SetPlayerAt(int16_t x, int16_t y, bool t);

    int16_t getMinX() const;
    int16_t getMinY() const;
    int16_t getMaxX() const;
    int16_t getMaxY() const;
    int16_t getLevel() const;
    uint16_t getWidth() const;
    uint16_t getHeight() const;

    bool addItemToPos(Item it, MapPosition pos);
    bool addContainerToPos(Item it, Container *cc, MapPosition pos);
    bool addAlwaysContainerToPos(Item it, Container *cc, MapPosition pos);

    bool intersects(const position &origin, uint16_t width,
                    uint16_t height) const;
    bool intersects(const Map &map) const;

    struct FieldNotFound : std::exception {};

private:
    void ageItems();
    void ageContainers();

    inline uint16_t Conv_X_Koord(int16_t x);
    inline uint16_t Conv_Y_Koord(int16_t y);
    inline int16_t Conv_To_X(uint16_t x);
    inline int16_t Conv_To_Y(uint16_t y);
};

#endif
