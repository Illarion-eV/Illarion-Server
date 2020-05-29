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


#ifndef globals_HH
#define globals_HH

#include <exception>
#include <map>
#include <iostream>
#include <boost/functional/hash/hash.hpp>

#include "types.hpp"
#include "constants.hpp"

struct FieldNotFound : std::exception {};
struct MapError : std::exception {};

struct position {
    short int x;
    short int y;
    short int z;

    position() = default;
    position(short int x, short int y, short int z) : x(x), y(y), z(z) {}
    
    auto operator == (const position &pos) const -> bool {
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

    auto toString() const -> std::string {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }

    friend auto operator<<(std::ostream &out, const position &pos) -> std::ostream & {
        out << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
        return out;
    }

    friend auto hash_value(const position &p) -> std::size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        boost::hash_combine(seed, p.z);

        return seed;
    }
};

struct PositionComparison {
    auto operator()(const position& pos1, const position& pos2) const -> bool {
        if(pos1.x == pos2.x) {
            if(pos1.y == pos2.y) {
                return pos1.z < pos2.z;
            } else {
                return pos1.y < pos2.y;
            }
        } else {
            return pos1.x < pos2.x;
        }
    }
};

namespace std {
template<> struct hash<position> {
    auto operator()(const position &p) const -> size_t {
        return hash_value(p);
    }
};
}

struct MapPosition {
    short int x;
    short int y;

    MapPosition() = default;
    MapPosition(short int x, short int y): x(x), y(y) {}
    explicit MapPosition(const position &pos): x(pos.x), y(pos.y) {}

    auto operator == (const MapPosition &pos) const -> bool {
        return (x == pos.x && y == pos.y);
    }

    friend auto hash_value(const MapPosition &p) -> std::size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);

        return seed;
    }
};

namespace std {
template<> struct hash<MapPosition> {
    auto operator()(const MapPosition &p) const -> size_t {
        return hash_value(p);
    }
};
}

struct Range {
    int radius = 0;
    int zRadius = RANGEUP;
};

enum QuestAvailability {
    questDefaultAvailable = 0,
    questAvailable = 1,
    questWillBeAvailable = 2,
    questNotAvailable = 3
};

using script_data_exchangemap = std::vector<std::pair<std::string, std::string>>;

#endif
