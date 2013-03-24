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


#ifndef globals_HH
#define globals_HH

#include <map>
#include <fstream>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <boost/functional/hash/hash.hpp>
#include "types.hpp"

struct position {
    short int x;
    short int y;
    short int z;

    position() = default;
    position(short int x, short int y, short int z) : x(x), y(y), z(z) {}
    
    bool operator == (const position &pos) const {
        return (x == pos.x && y == pos.y && z == pos.z);
    }

    void move(direction dir) {
        switch (dir) {
        case dir_north:
            --y;
            break;
        case dir_northeast:
            --y;
            ++x;
            break;
        case dir_east:
            ++x;
            break;
        case dir_southeast:
            ++y;
            ++x;
            break;
        case dir_south:
            ++y;
            break;
        case dir_southwest:
            ++y;
            --x;
            break;
        case dir_west:
            --x;
            break;
        case dir_northwest:
            --y;
            --x;
            break;
        case dir_up:
            ++z;
            break;
        case dir_down:
            --z;
            break;
        default:
            break;
        }
    }

    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    friend std::ostream &operator<<(std::ostream &out, const position &pos) {
        out << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        return out;
    }

    friend std::size_t hash_value(const position &p) {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        boost::hash_combine(seed, p.z);

        return seed;
    }
};

namespace std {
template<> struct hash<position> {
    size_t operator()(const position &p) const {
        return hash_value(p);
    }
};
}

struct MAP_POSITION {
    short int x;
    short int y;

    MAP_POSITION() = default;
    MAP_POSITION(short int x, short int y): x(x), y(y) {}
    MAP_POSITION(const position &pos): x(pos.x), y(pos.y) {}

    bool operator == (const MAP_POSITION &pos) const {
        return (x == pos.x && y == pos.y);
    }

    friend std::size_t hash_value(const MAP_POSITION &p) {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);

        return seed;
    }
};

namespace std {
template<> struct hash<MAP_POSITION> {
    size_t operator()(const MAP_POSITION &p) const {
        return hash_value(p);
    }
};
}

typedef std::vector<std::pair<std::string, std::string>> script_data_exchangemap;

#endif
