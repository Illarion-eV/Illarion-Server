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

/**
 * @brief Provides thread-safe random number generation using Mersenne Twister.
 *
 * This class offers static methods for generating random numbers with uniform
 * and normal distributions. All methods use a shared Mersenne Twister engine
 * for high-quality pseudorandom number generation.
 */
class Random {
private:
    static std::mt19937 rng; ///< Mersenne Twister random number generator engine.

public:
    /**
     * @brief Generates a uniformly distributed random number in [0.0, 1.0).
     * @return A random double value between 0.0 (inclusive) and 1.0 (exclusive).
     * @note Uses std::uniform_real_distribution for high-quality random doubles.
     */
    static auto uniform() -> double;

    /**
     * @brief Generates a normally distributed random number.
     * @param mean The mean (average) of the normal distribution.
     * @param sd The standard deviation of the normal distribution.
     * @return A random double value from the specified normal distribution.
     */
    static auto normal(double mean, double sd) -> double;

    /**
     * @brief Generates a uniformly distributed random integer in [min, max].
     * @tparam IntType An integer type (signed or unsigned).
     * @param min The minimum value (inclusive).
     * @param max The maximum value (inclusive).
     * @return A random integer between min and max (both inclusive).
     * @throws std::invalid_argument if min > max.
     */
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

    /**
     * @brief Generates a uniformly distributed random integer in [0, count-1].
     * @tparam IntType An unsigned integer type.
     * @param count The number of possible values (upper bound is count-1).
     * @return A random integer between 0 (inclusive) and count-1 (inclusive).
     * @note This is a convenience method for generating array indices or selecting
     *       from a fixed number of options.
     */
    template <class IntType> static auto uniform(IntType count) -> IntType {
        static_assert(std::is_unsigned_v<IntType>);
        return uniform(IntType{0}, count - 1);
    }
};

#endif
