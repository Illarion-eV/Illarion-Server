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

#include "constants.hpp"
#include "types.hpp"

#include <boost/functional/hash/hash.hpp>
#include <charconv>
#include <exception>
#include <iostream>
#include <map>

/**
 * @brief Exception thrown when a requested field does not exist on the map.
 */
struct FieldNotFound : std::exception {};

/**
 * @brief Exception thrown when a map operation encounters an error.
 */
struct MapError : std::exception {};

/**
 * @brief Represents a 3D coordinate in the game world.
 * 
 * Contains x, y coordinates on the map plane and z for elevation/level.
 * Provides movement and comparison operations for spatial calculations.
 */
struct position {
    Coordinate x;
    Coordinate y;
    Coordinate z;

    position() = default;
    position(Coordinate x, Coordinate y, Coordinate z) : x(x), y(y), z(z) {}

    auto operator==(const position &pos) const -> bool { return (x == pos.x && y == pos.y && z == pos.z); }

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

    [[nodiscard]] auto toString() const -> std::string {
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

/**
 * @brief Comparator for position objects to enable use in sorted containers.
 * 
 * Orders positions lexicographically by x, then y, then z coordinates.
 */
struct PositionComparison {
    auto operator()(const position &pos1, const position &pos2) const -> bool {
        if (pos1.x == pos2.x) {
            if (pos1.y == pos2.y) {
                return pos1.z < pos2.z;
            }
            return pos1.y < pos2.y;
        }
        return pos1.x < pos2.x;
    }
};

/**
 * @brief Standard library namespace.
 */
namespace std {

/**
 * @brief Hash specialization for position to enable use in unordered containers.
 * 
 * Allows position objects to be keys in std::unordered_map and std::unordered_set.
 */
template <> struct hash<position> {
    auto operator()(const position &p) const -> size_t { return hash_value(p); }
};
} // namespace std

/**
 * @brief Represents a 2D coordinate on the map plane.
 * 
 * Similar to position but without elevation (z coordinate). Used for map-level
 * operations that don't require height information.
 */
struct MapPosition {
    Coordinate x;
    Coordinate y;

    MapPosition() = default;
    MapPosition(Coordinate x, Coordinate y) : x(x), y(y) {}
    explicit MapPosition(const position &pos) : x(pos.x), y(pos.y) {}

    auto operator==(const MapPosition &pos) const -> bool { return (x == pos.x && y == pos.y); }

    friend auto hash_value(const MapPosition &p) -> std::size_t {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);

        return seed;
    }
};

namespace std {

/**
 * @brief Hash specialization for MapPosition to enable use in unordered containers.
 * 
 * Allows MapPosition objects to be keys in std::unordered_map and std::unordered_set.
 */
template <> struct hash<MapPosition> {
    auto operator()(const MapPosition &p) const -> size_t { return hash_value(p); }
};
} // namespace std

/**
 * @brief Defines a spatial range with horizontal and vertical radii.
 * 
 * Used for area-of-effect calculations, visibility checks, and range queries.
 */
struct Range {
    Coordinate radius = 0;
    Coordinate zRadius = RANGEUP;
};

enum QuestAvailability {
    questDefaultAvailable = 0,
    questAvailable = 1,
    questWillBeAvailable = 2,
    questNotAvailable = 3
};

using script_data_exchangemap = std::vector<std::pair<std::string, std::string>>;

template <typename T> auto stringToNumber(const std::string &s, T &n) -> bool {
    auto [p, error] = std::from_chars(s.data(), s.data() + s.length(), n); // NOLINT
    return error == std::errc();
}

#endif
