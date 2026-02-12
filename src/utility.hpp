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

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "types.hpp"

/**
 * @brief Case-insensitive character comparison predicate.
 * @param c1 First character to compare.
 * @param c2 Second character to compare.
 * @return true if characters are equal (ignoring case), false otherwise.
 */
extern auto mypred(char c1, char c2) -> bool;

/**
 * @brief Case-insensitive string comparison.
 * @param s1 First string to compare.
 * @param s2 Second string to compare.
 * @return true if strings are equal (ignoring case), false otherwise.
 */
extern auto comparestrings_nocase(const std::string &s1, const std::string &s2) -> bool;

/**
 * @brief Converts a numeric direction value to a direction enum.
 * @param dir The numeric direction value to convert.
 * @return The corresponding direction enum value, or dir_none if out of range.
 */
extern auto to_direction(uint8_t dir) -> direction;

/**
 * @brief Checks if a string contains only numeric digits.
 * @param str The string to check.
 * @return true if the string is non-empty and contains only digits 0-9, false otherwise.
 */
extern auto isNumeric(const std::string &str) -> bool;

/**
 * @brief A simple iterator range wrapper for use with range-based for loops.
 * @tparam T An iterator type.
 */
template <class T> struct iterator_range {
    /**
     * @brief Returns the beginning iterator.
     * @return The first iterator in the range.
     */
    [[nodiscard]] auto begin() const -> T { return p.first; }

    /**
     * @brief Returns the ending iterator.
     * @return The last iterator in the range.
     */
    [[nodiscard]] auto end() const -> T { return p.second; }

    std::pair<T, T> p; ///< The pair of iterators defining the range.
};

#endif
