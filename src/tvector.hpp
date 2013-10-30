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


// thread safe list to pass objects from one thread to another...
// only use push_back, pop_front or clear!
// note: pop_front returns the object as well as deleting it from the list...

#ifndef __tvector_hpp
#define __tvector_hpp

#include <list>
#include <exception>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

template<class T> class tvector : public std::list<T> {
public:
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

    inline T &pop_front() {
        std::lock_guard<std::mutex> lock(vlock);
        T &item = std::list<T>::front();
        std::list<T>::pop_front();
        return item;
    }

private:
    std::mutex vlock;
};

#endif
