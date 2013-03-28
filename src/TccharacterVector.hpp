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


#ifndef TCCHARACTERVECTOR_HH
#define TCCHARACTERVECTOR_HH

#include <string>
#include <vector>
#include "utility.hpp"
#include "constants.hpp"

template < class _Tp >
class ccharactervector : public std::vector < _Tp > {
public:
    _Tp find(const std::string &name) const;
    _Tp findID(TYPE_OF_CHARACTER_ID id) const;
    _Tp find(const position &pos) const;
    bool find(const position &pos, _Tp &ret) const;
    
    bool remove(const position &pos);

    bool getIterator(TYPE_OF_CHARACTER_ID id, typename ccharactervector::iterator &newIt);

    std::vector < _Tp > findAllCharactersInRangeOf(const position &pos, int distancemetric) const;
    std::vector < _Tp > findAllCharactersInMaxRangeOf(const position &pos, int distancemetric) const;
    std::vector < _Tp > findAllCharactersInScreen(const position &pos) const;
    std::vector < _Tp > findAllAliveCharactersInRangeOf(const position &pos, int distancemetric) const;
    std::vector < _Tp > findAllAliveCharactersInRangeOfOnSameMap(const position &pos, int distancemetric) const;
    bool findAllCharactersWithXInRangeOf(short int startx, short int endx, std::vector < _Tp > &ret) const;

};


template < class _Tp > _Tp ccharactervector < _Tp > ::find(const std::string &n) const {
    for (const auto &character : *this) {
        if (comparestrings_nocase(character->getName(), n)) {
            return character;
        }
    }

    return nullptr;
}


template < class _Tp > _Tp ccharactervector < _Tp > ::findID(TYPE_OF_CHARACTER_ID id) const {
    for (const auto &character : *this) {
        if (character->getId() == id) {
            return character;
        }
    }

    return nullptr;
}


template < class _Tp > _Tp ccharactervector < _Tp > ::find(const position &pos) const {
    for (const auto &character : *this) {
        if (character->pos == pos) {
            return character;
        }
    }

    return nullptr;
}


template < class _Tp > bool ccharactervector < _Tp > ::find(const position &pos, _Tp &ret) const {
    for (const auto &character : *this) {
        if (character->pos == pos) {
            ret = character;
            return true;
        }
    }

    return false;
}


template < class _Tp > bool ccharactervector < _Tp > ::remove(const position &pos) {
    for (auto thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator) {
        if ((*thisIterator)->pos == pos) {
            erase(thisIterator);
            return true;
        }
    }

    return false;
}


template < class _Tp > bool ccharactervector < _Tp > ::getIterator(TYPE_OF_CHARACTER_ID id, typename ccharactervector::iterator &newIt) {
    for (auto thisIterator = this->begin(); thisIterator < this->end(); ++thisIterator) {
        if ((*thisIterator)->getId() == id) {
            newIt = thisIterator;
            return true;
        }
    }

    return false;

}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllCharactersInRangeOf(const position &pos, int distancemetric) const {
    std::vector < _Tp > temp;

    for (const auto &character : *this) {
        short int dz = character->pos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = character->pos.x - pos.x;
            short int dy = character->pos.y - pos.y;

            if ((abs(dx) + abs(dy)) <= distancemetric) {
                temp.push_back(character);
            }
        }
    }

    return temp;
}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllCharactersInScreen(const position &pos) const {
    std::vector < _Tp > temp;

    for (const auto &character : *this) {
        short int dz = character->pos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = character->pos.x - pos.x;
            short int dy = character->pos.y - pos.y;

            if ((abs(dx) + abs(dy)) <= character->getScreenRange()) {
                temp.push_back(character);
            }
        }
    }

    return temp;
}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllCharactersInMaxRangeOf(const position &pos, int distancemetric) const {
    std::vector < _Tp > temp;

    for (const auto &character : *this) {
        short int dz = character->pos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = character->pos.x - pos.x;
            short int dy = character->pos.y - pos.y;

            if ((abs(dx) <= distancemetric) && (abs(dy) <=distancemetric)) {
                temp.push_back(character);
            }
        }
    }

    return temp;
}


template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllAliveCharactersInRangeOf(const position &pos, int distancemetric) const {
    std::vector < _Tp > temp;

    for (const auto &character : *this) {
        short int dz = character->pos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = character->pos.x - pos.x;
            short int dy = character->pos.y - pos.y;

            if (((abs(dx) + abs(dy)) <= distancemetric) ||
                ((distancemetric == 1) && (abs(dx) == 1) && (abs(dy) == 1))) {       // Allow angle attacks
                if (character->IsAlive()) {
                    temp.push_back(character);
                }
            }
        }
    }

    return temp;
}

template < class _Tp > std::vector < _Tp > ccharactervector < _Tp > ::findAllAliveCharactersInRangeOfOnSameMap(const position &pos, int distancemetric) const {
    std::vector < _Tp > temp;

    for (const auto &character : *this) {
        if (character->pos.z == pos.z) {
            short int dx = character->pos.x - pos.x;
            short int dy = character->pos.y - pos.y;

            if (((abs(dx) + abs(dy)) <= distancemetric) || ((distancemetric == 1) && (abs(dx) == 1) && (abs(dy) == 1))) {          // Allow angle attacks
                if (character->IsAlive()) {
                    temp.push_back(character);
                }
            }
        }
    }

    return temp;
}


template < class _Tp > bool ccharactervector < _Tp > ::findAllCharactersWithXInRangeOf(short int startx, short int endx, std::vector < _Tp > &ret) const {
    bool found_one = false;

    for (const auto &character : *this) {
        if ((character->pos.x >= startx) && (character->pos.x <= endx)) {
            ret.push_back(character);
            found_one = true;
        }
    }

    return found_one;
}

#endif
