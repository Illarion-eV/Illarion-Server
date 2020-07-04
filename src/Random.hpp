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

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>
#include <sstream>
#include <stdexcept>
#include <type_traits>

class Random {
private:
    static std::mt19937 rng;

public:
    static auto uniform() -> double;
    static auto normal(double mean, double sd) -> double;

    template <class IntType> static auto uniform(IntType min, IntType max) -> IntType {
        static_assert(std::is_same_v<IntType, short> || std::is_same_v<IntType, int> || std::is_same_v<IntType, long> ||
                      std::is_same_v<IntType, long long> || std::is_same_v<IntType, unsigned short> ||
                      std::is_same_v<IntType, unsigned int> || std::is_same_v<IntType, unsigned long> ||
                      std::is_same_v<IntType, unsigned long long>);
        if (max < min) {
            std::stringstream error;
            error << "Random::uniform: Invalid arguments, min(" << min << ") > max(" << max << ")";
            throw std::invalid_argument(error.str());
        }

        std::uniform_int_distribution<IntType> uniform(min, max);
        return uniform(rng);
    }

    template <class IntType> static auto uniform(IntType count) -> IntType {
        static_assert(std::is_unsigned_v<IntType>);
        return uniform(IntType{0}, count - 1);
    }
};

#endif
