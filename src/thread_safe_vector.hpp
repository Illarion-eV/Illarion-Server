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


#ifndef __thread_safe_vector_hpp
#define __thread_safe_vector_hpp

#include <list>
#include <exception>
#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <chrono>

template<class T> class thread_safe_vector : public std::list<T> {
public:
    inline size_t size() {
        std::lock_guard<std::mutex> lock(vlock);
        uint16_t s = std::list<T>::size();
        return s;
    }

    inline void clear() {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::clear();
    }

    inline void push_back(const T &item) {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::push_back(item);
    }

    inline bool empty() {
        std::lock_guard<std::mutex> lock(vlock);
        return std::list<T>::empty();
    }

    inline T pop_front() {
        std::lock_guard<std::mutex> lock(vlock);
        T item = std::list<T>::front();
        std::list<T>::pop_front();
        return item;
    }

private:
    std::mutex vlock;
};

#endif
