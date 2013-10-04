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


#ifndef __tpvector_hpp
#define __tpvector_hpp

#include <list>
#include <exception>
#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>
#include <chrono>

#define _MAX_NON_BLOCK_TRYS_ 20

template<class T> class tpvector : public std::list<T> {
public:
    inline size_t size() {
        std::lock_guard<std::mutex> lock(vlock);
        uint16_t s = std::list<T>::size();
        return s;
    }

    inline size_t non_block_size() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            if (!vlock.try_lock()) {
                std::chrono::milliseconds wait(5);
                std::cout<<"non_block_size blocked mutex"<<std::endl;
                std::this_thread::sleep_for(wait);
            } else {
                auto s = std::list<T>::size();
                vlock.unlock();
                return s;
            }
        }

        return 0;
    }

    inline void clear() {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::clear();
    }

    inline void push_back(const T &item) {
        std::lock_guard<std::mutex> lock(vlock);
        std::list<T>::push_back(item);
    }

    inline bool non_block_push_back(const T &item) {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            if (!vlock.try_lock()) {
                std::chrono::milliseconds wait(5);
                std::cout<<"non_block_push_back blocked mutex"<<std::endl;
                std::this_thread::sleep_for(wait);
            } else {
                std::list<T>::push_back(item);
                vlock.unlock();
                return true;
            }
        }

        return false;
    }

    inline bool non_block_push_front(const T &item) {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            if (!vlock.try_lock()) {
                std::chrono::milliseconds wait(5);
                std::cout<<"non_block_push_front blocked mutex"<<std::endl;
                std::this_thread::sleep_for(wait);
            } else {
                std::list<T>::push_front(item);
                vlock.unlock();
                return true;
            }
        }

        return false;
    }

    inline T non_block_pop_front() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            if (!vlock.try_lock()) {
                std::chrono::milliseconds wait(5);
                std::cout<<"non_block_pop_front blocked mutex"<<std::endl;
                std::this_thread::sleep_for(wait);
            } else {
                T item = std::list<T>::front();
                std::list<T>::pop_front();
                vlock.unlock();
                return item;
            }
        }

        T item;
        return item;

    }



    inline bool empty() {
        std::lock_guard<std::mutex> lock(vlock);
        return std::list<T>::empty();
    }

    inline bool non_block_empty() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            if (!vlock.try_lock()) {
                std::chrono::milliseconds wait(5);
                std::cout<<"non_block_empty blocked mutex"<<std::endl;
                std::this_thread::sleep_for(wait);
            } else {
                bool empty = std::list<T>::empty();
                vlock.unlock();
                return empty;
            }
        }

        return true;
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
