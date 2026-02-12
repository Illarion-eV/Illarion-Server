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

#ifndef THREAD_SAFE_VECTOR_HPP
#define THREAD_SAFE_VECTOR_HPP

#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>

/**
 * @brief Thread-safe wrapper around std::list providing synchronized access.
 * @tparam T Element type stored in the list
 *
 * Provides a subset of std::list operations with automatic mutex locking
 * to ensure thread-safe concurrent access. All operations use std::lock_guard
 * for exception-safe locking.
 *
 * Supported operations:
 * - size(): Get number of elements
 * - empty(): Check if list is empty
 * - clear(): Remove all elements
 * - push_back(): Add element to end
 * - pop_front(): Remove and return first element
 *
 * @note Despite the name, this is actually based on std::list, not std::vector
 * @note pop_front() will throw if called on empty list (inherited from std::list)
 */
template <class T> class thread_safe_vector : public std::list<T> {
public:
    /**
     * @brief Returns number of elements with thread safety.
     * @return Number of elements in the list
     */
    inline auto size() -> size_t {
        std::lock_guard<std::mutex> lock(vlock);
        uint16_t s = std::list<T>::size();
        return s;
    }

    /**
     * @brief Removes all elements with thread safety.
     */
    inline void clear() {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::clear();
    }

    /**
     * @brief Appends element to end of list with thread safety.
     * @param item Element to add
     */
    inline void push_back(const T &item) {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::push_back(item);
    }

    /**
     * @brief Checks if list is empty with thread safety.
     * @return True if list contains no elements
     */
    inline auto empty() -> bool {
        std::lock_guard<std::mutex> lock(vlock);
        return std::list<T>::empty();
    }

    /**
     * @brief Removes and returns first element with thread safety.
     * @return Copy of the first element
     * @throws std::exception if list is empty
     */
    inline auto pop_front() -> T {
        std::lock_guard<std::mutex> lock(vlock);
        T item = std::list<T>::front();
        std::list<T>::pop_front();
        return item;
    }

private:
    std::mutex vlock; ///< Mutex for thread-safe access to list operations
};

#endif
