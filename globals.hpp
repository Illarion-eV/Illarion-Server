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
#include <boost/functional/hash_fwd.hpp>
#include "types.hpp"

/**
* @ingroup Scriptclasses
* defines a 3d coordinate in the area
* exportet to lua as <b>position</b>
*/
struct position {
    /**
    *@name position Lua Variables:
    * to lua exportet variables
    */

    //@{
    /**
    *=============================start group Lua Variables===================
    *@ingroup Scriptvariables
    */

    /**
    * x-coordinate of the position
    * <b>Lua: (r/w) [x]</b>
    */
    short int x;

    /**
    * y-coordinate of the position
    * <b>Lua: (r/w) [y]</b>
    */
    short int y;

    /**
    * z-coordinate of the position
    * <b>Lua: (r/w) [z]</b>
    */
    short int z;

    /**
    *=============================end group Lua Variables===================
    */
    //@}


    /**
    * @name position Lua Functions:
    * to Lua exported Functions
    */

    //@{
    /**
    *=============================start group Lua functions===================
    *@ingroup Scriptfunctions
    */

    /**
    * standard constructor for creating a new position
    * @param _x starting x position of the coordinate
    * @param _y starting y position of the coordinate
    * @param _z starting z position of the coordinate
    */
    position(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

    /**
    * standard constructor which creates a position and initializes x,y,z with 0
    */
    position() : x(0), y(0), z(0) {}

    /**
    *=============================end Lua Variables===================
    */

    //@}


    /**
    * overloaded == operator which determines if one position is equal to another one
    */
    bool operator == (const position &pos) const {
        return (x == pos.x && y == pos.y && z == pos.z);
    }

    std::string toString() const {
        return "(" + boost::lexical_cast<std::string>(x) + ", " + boost::lexical_cast<std::string>(y) + ", " + boost::lexical_cast<std::string>(z) + ")";
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

/**
* determines a 2d Postion on one single map
* the z coordinate is the map itself
*/
struct MAP_POSITION {
    short int x;
    short int y;

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

// a map storing configuration options from a config file...
extern std::map<std::string, std::string> configOptions;

//! a log file for kills
extern std::ofstream kill_log;

#endif
