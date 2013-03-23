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


#ifndef __tpvector_hpp
#define __tpvector_hpp

#include <list>
#include <exception>
#include <iostream>
#include <cerrno> // error numbers.. EAGAIN etc.
#include <pthread.h>
#include <stdint.h>

#define _MAX_NON_BLOCK_TRYS_ 20

template<class T> class tpvector : public std::list<T> {
public:
    tpvector() : std::list<T>() {
        // initialize our mutex
        vlock = new pthread_mutex_t;

        if (pthread_mutex_init(vlock,NULL)) {
            std::cout << "Mutex couldn't get initialized... throwing exception!" << std::endl;
            throw std::exception();
        }
    }

    ~tpvector() {
        pthread_mutex_destroy(vlock);
        delete vlock;
    }

    inline uint16_t size() {
        // get mutex
        if (pthread_mutex_lock(vlock)) {
            std::cout << "problem with mutex locking!" << std::endl;
        }

        uint16_t s = std::list<T>::size();

        // release mutex
        if (pthread_mutex_unlock(vlock)) {
            std::cout << "problem with mutex unlocking occured!" << std::endl;
        }

        return s;
    }

    inline uint16_t non_block_size() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            int err = pthread_mutex_trylock(vlock);

            if (err) {
                timespec stime;
                stime.tv_sec = 0;
                stime.tv_nsec = 5000000;
                std::cout<<"non_block_size blocked mutex"<<std::endl;
                nanosleep(&stime, NULL);
            } else {
                uint16_t s = std::list<T>::size();
                pthread_mutex_unlock(vlock);
                return s;
            }
        }

        return 0;
    }




    inline void clear() {
        // get mutex
        if (pthread_mutex_lock(vlock)) {
            std::cout << "problem with mutex locking!" << std::endl;
        }

        std::list<T>::clear();

        // release mutex
        if (pthread_mutex_unlock(vlock)) {
            std::cout << "problem with mutex unlocking occured!" << std::endl;
        }
    }

    inline void push_back(const T &item) {
        // get mutex
        if (pthread_mutex_lock(vlock)) {
            std::cout << "problem with mutex locking!" << std::endl;
        }

        std::list<T>::push_back(item);

        // release mutex
        if (pthread_mutex_unlock(vlock)) {
            std::cout << "problem with mutex unlocking occured!" << std::endl;
        }
    }

    inline bool non_block_push_back(const T &item) {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            int err = pthread_mutex_trylock(vlock);

            if (err) {
                timespec stime;
                stime.tv_sec = 0;
                stime.tv_nsec = 5000000;
                std::cout<<"non_block_push_back blocked mutex"<<std::endl;
                nanosleep(&stime, NULL);
            } else {
                std::list<T>::push_back(item);
                pthread_mutex_unlock(vlock);
                return true;
            }
        }

        return false;
    }

    inline bool non_block_push_front(const T &item) {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            int err = pthread_mutex_trylock(vlock);

            if (err) {
                timespec stime;
                stime.tv_sec = 0;
                stime.tv_nsec = 5000000;
                std::cout<<"non_block_push_front blocked mutex"<<std::endl;
                nanosleep(&stime, NULL);
            } else {
                std::list<T>::push_front(item);
                pthread_mutex_unlock(vlock);
                return true;
            }
        }

        return false;
    }

    inline T non_block_pop_front() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            int err = pthread_mutex_trylock(vlock);

            if (err) {
                timespec stime;
                stime.tv_sec = 0;
                stime.tv_nsec = 5000000;
                std::cout<<"non_block_push_back blocked mutex"<<std::endl;
                nanosleep(&stime, NULL);
            } else {
                T item = std::list<T>::front();
                std::list<T>::pop_front();
                pthread_mutex_unlock(vlock);
                return item;
            }
        }

        T item;
        return item;

    }



    inline bool empty() {
        if (pthread_mutex_lock(vlock)) {
            std::cout << "problem with mutex locking!" << std::endl;
        }

        bool empty = std::list<T>::empty();

        // release mutex
        if (pthread_mutex_unlock(vlock)) {
            std::cout << "problem with mutex unlocking occured!" << std::endl;
        }

        return empty;
    }

    inline bool non_block_empty() {
        for (int i = 0; i < _MAX_NON_BLOCK_TRYS_ ; ++i) {
            int err = pthread_mutex_trylock(vlock);

            if (err) {
                timespec stime;
                stime.tv_sec = 0;
                stime.tv_nsec = 5000000;
                std::cout<<"non_block_empty blocked mutex"<<std::endl;
                nanosleep(&stime, NULL);
            } else {
                bool empty = std::list<T>::empty();
                pthread_mutex_unlock(vlock);
                return empty;
            }
        }

        return true;
    }

    inline T pop_front() {
        // get mutex
        if (pthread_mutex_lock(vlock)) {
            std::cout << "problem with mutex locking!" << std::endl;
        }

        T item = std::list<T>::front();
        std::list<T>::pop_front();

        // release mutex
        if (pthread_mutex_unlock(vlock)) {
            std::cout << "problem with mutex unlocking occured!" << std::endl;
        }

        return item;
    }

private:
    pthread_mutex_t *vlock;

};

#endif
