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

#include "utility.hpp"

#include <algorithm>
#include <cctype>
#include <string>

auto mypred(char c1, char c2) -> bool {
    return tolower(c1) == tolower(c2);
}

auto comparestrings_nocase(const std::string &s1, const std::string &s2) -> bool {
    if (s1.size() != s2.size()) {
        return false;
    }

    return equal(s1.begin(), s1.end(), s2.begin(), mypred);
}

auto to_direction(uint8_t dir) -> direction {
    if (dir < 10) {
        return static_cast<direction>(dir);
    } else {
        return dir_none;
    }
}
