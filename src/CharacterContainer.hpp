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


#ifndef _CHARACTERCONTAINER_HPP_
#define _CHARACTERCONTAINER_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include "utility.hpp"
#include "constants.hpp"

template <class T>
class CharacterContainer {
public:
    typedef T* pointer;

private:
    typedef std::function<void(pointer)> for_each_type;
    typedef void(T::*for_each_member_type)();
    typedef typename std::unordered_map<TYPE_OF_CHARACTER_ID, pointer> container_type;
    container_type container;

public:
    bool empty() const {
        return container.empty();
    }

    auto size() const -> decltype(container.size()) {
        return container.size();
    }

    void insert(pointer p) {
        container.emplace(p->getId(), p);
    }

    pointer find(const std::string &name) const;
    pointer find(TYPE_OF_CHARACTER_ID id) const;
    pointer find(const position &pos) const;
    
    bool erase(TYPE_OF_CHARACTER_ID id);
    void clear() {
        container.clear();
    }

    std::vector<pointer> findAllCharactersInRangeOf(const position &pos, int distancemetric) const;
    std::vector<pointer> findAllCharactersInMaxRangeOf(const position &pos, int distancemetric) const;
    std::vector<pointer> findAllCharactersInScreen(const position &pos) const;
    std::vector<pointer> findAllAliveCharactersInRangeOf(const position &pos, int distancemetric) const;
    std::vector<pointer> findAllAliveCharactersInRangeOfOnSameMap(const position &pos, int distancemetric) const;
    bool findAllCharactersWithXInRangeOf(short int startx, short int endx, std::vector<pointer> &ret) const;

    void for_each(const for_each_type &function) {
        for (const auto &key_value : container) {
            function(key_value.second);
        }
    }

    void for_each(const for_each_type &function) const {
        for (const auto &key_value : container) {
            function(key_value.second);
        }
    }

    void for_each(const for_each_member_type &function) {
        for (const auto &key_value : container) {
            (key_value.second->*function)();
        }
    }
};


template <class T>
auto CharacterContainer<T>::find(const std::string &n) const -> pointer {
    for (const auto &character : container) {
        if (comparestrings_nocase(character.second->getName(), n)) {
            return character.second;
        }
    }

    return nullptr;
}


template <class T>
auto CharacterContainer<T>::find(TYPE_OF_CHARACTER_ID id) const -> pointer {
    const auto it = container.find(id);

    if (it != container.end()) {
        return it->second;
    }

    return nullptr;
}


template <class T>
auto CharacterContainer<T>::find(const position &pos) const -> pointer {
    for (const auto &character : container) {
        if (character.second->getPosition() == pos) {
            return character.second;
        }
    }

    return nullptr;
}


template <class T>
bool CharacterContainer<T>::erase(TYPE_OF_CHARACTER_ID id) {
    return container.erase(id) > 0;
}


template <class T>
auto CharacterContainer<T>::findAllCharactersInRangeOf(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();
        short int dz = charPos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = charPos.x - pos.x;
            short int dy = charPos.y - pos.y;

            if ((abs(dx) + abs(dy)) <= distancemetric) {
                temp.push_back(character.second);
            }
        }
    }

    return temp;
}


template <class T>
auto CharacterContainer<T>::findAllCharactersInScreen(const position &pos) const -> std::vector<pointer> {
    std::vector<pointer> temp;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();
        short int dz = charPos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = charPos.x - pos.x;
            short int dy = charPos.y - pos.y;

            if ((abs(dx) + abs(dy)) <= character.second->getScreenRange()) {
                temp.push_back(character.second);
            }
        }
    }

    return temp;
}


template <class T>
auto CharacterContainer<T>::findAllCharactersInMaxRangeOf(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();
        short int dz = charPos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = charPos.x - pos.x;
            short int dy = charPos.y - pos.y;

            if ((abs(dx) <= distancemetric) && (abs(dy) <=distancemetric)) {
                temp.push_back(character.second);
            }
        }
    }

    return temp;
}


template <class T>
auto CharacterContainer<T>::findAllAliveCharactersInRangeOf(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();
        short int dz = charPos.z - pos.z;

        if ((-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
            short int dx = charPos.x - pos.x;
            short int dy = charPos.y - pos.y;

            if (((abs(dx) + abs(dy)) <= distancemetric) ||
                ((distancemetric == 1) && (abs(dx) == 1) && (abs(dy) == 1))) {       // Allow angle attacks
                if (character.second->IsAlive()) {
                    temp.push_back(character.second);
                }
            }
        }
    }

    return temp;
}

template <class T>
auto CharacterContainer<T>::findAllAliveCharactersInRangeOfOnSameMap(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();

        if (charPos.z == pos.z) {
            short int dx = charPos.x - pos.x;
            short int dy = charPos.y - pos.y;

            if (((abs(dx) + abs(dy)) <= distancemetric) || ((distancemetric == 1) && (abs(dx) == 1) && (abs(dy) == 1))) {          // Allow angle attacks
                if (character.second->IsAlive()) {
                    temp.push_back(character.second);
                }
            }
        }
    }

    return temp;
}


template <class T>
bool CharacterContainer<T>::findAllCharactersWithXInRangeOf(short int startx, short int endx, std::vector<pointer> &ret) const {
    bool found_one = false;

    for (const auto &character : container) {
        const auto &charPos = character.second->getPosition();

        if ((charPos.x >= startx) && (charPos.x <= endx)) {
            ret.push_back(character.second);
            found_one = true;
        }
    }

    return found_one;
}

#endif
