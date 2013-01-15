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

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <stdexcept>

template<class T>
class fuse_ptr {
public:
    typedef boost::unordered_map<T *, boost::unordered_set<fuse_ptr<T>*> > fusebox_t;

private:
    T **ptr;
    static fusebox_t fusebox;

public:
    fuse_ptr() {
        ptr = new T*;
        *ptr = 0;
    }

    fuse_ptr(T *p) {
        ptr = new T*;
        *ptr = p;

        if (p != 0) {
            fusebox[p].insert(this);
        }
    }

    fuse_ptr(fuse_ptr const &p) {
        ptr = new T*;
        *ptr = *(p.ptr);
        fusebox[*ptr].insert(this);
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
            auto it = fusebox.find(*ptr);

            if (it != fusebox.end()) {
                it->second.erase(this);
            }
        }

        delete ptr;
        ptr = 0;
    }

    static void blow_fuse(T *p) {
        auto fuse_it = fusebox.find(p);

        if (fuse_it != fusebox.end()) {

            for (auto it = fuse_it->second.begin(); it != fuse_it->second.end(); ++it) {
                *((*it)->ptr) = 0;
            }

            fusebox.erase(fuse_it);
        }
    }
};

template<class T>
typename fuse_ptr<T>::fusebox_t fuse_ptr<T>::fusebox;

template<class T>
T *get_pointer(fuse_ptr<T> const &p) {
    return p.get();
}

template<class T>
bool isValid(fuse_ptr<T> const &p) {
    return (bool)p;
}

#endif

