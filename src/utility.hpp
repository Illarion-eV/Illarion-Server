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

#ifndef _UTILITY_HPP_
#define _UTILITY_HPP_

#include "types.hpp"

extern bool mypred(char c1, char c2);
extern bool comparestrings_nocase(const std::string &s1, const std::string &s2);
extern direction to_direction(uint8_t dir);


template <class T>
struct iterator_range {
    T begin() const {
        return p.first;
    }
    T end() const {
        return p.second;
    }
    std::pair<T,T> p;
};

#endif

