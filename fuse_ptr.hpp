/*
 *  illarionserver - server for the game Illarion
 *  Copyright 2011 Illarion e.V.
 *
 *  This file is part of illarionserver.
 *
 *  illarionserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  illarionserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FUSE_PTR_HPP_
#define _FUSE_PTR_HPP_

#include <map>
#include <stdexcept>

template<class T>
class fuse_ptr {
private:
    typedef std::pair<T*, fuse_ptr<T>*> Fuse;

    T **ptr;
    static std::multimap<T*, fuse_ptr<T>*> fusebox;

public:
    fuse_ptr() {
        ptr = new T*;
        *ptr = 0;
    }

    fuse_ptr(T *p) {
        ptr = new T*;
        *ptr = p;

        if (p != 0) {
            fusebox.insert(Fuse(p,this));
        }
    }

    fuse_ptr(fuse_ptr const &p) {
        ptr = new T*;
        *ptr = *(p.ptr);
        fusebox.insert(Fuse(*ptr,this));
    }

    T *get() const {
        if (*ptr == 0) {
            throw std::logic_error("Usage of invalid Character! Use isCharValid( char ) to check if a Character is still valid.");
        } else {
            return *ptr;
        }
    }

    operator T*() const {
        return get();
    }

    T *operator->() const {
        return get();
    }

    operator bool() const {
        return *ptr != 0;
    }

    virtual ~fuse_ptr() {
        if (*ptr != 0) {
            auto range = fusebox.equal_range(*ptr);
            auto it = range.first;

            for (; it != range.second && it->second != this; ++it);

            if (it != range.second) {
                fusebox.erase(it);
            }
        }

        delete ptr;
        ptr = 0;
    }

    static void blow_fuse(T *p) {
        auto range = fusebox.equal_range(p);

        for (auto it = range.first; it != range.second; ++it) {
            *(it->second->ptr) = 0;
        }

        fusebox.erase(p);
    }
};

template<class T>
std::multimap<T*, fuse_ptr<T>*> fuse_ptr<T>::fusebox;

template<class T>
T *get_pointer(fuse_ptr<T> const &p) {
    return p.get();
}

template<class T>
bool isValid(fuse_ptr<T> const &p) {
    return (bool)p;
}

#endif

