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

#include <map>
#include <unordered_map>

#include "globals.hpp"
#include "CharacterContainer.hpp"
#include "Player.hpp"
#include "Monster.hpp"
#include "NPC.hpp"


template <class T>
bool CharacterContainer<T>::getPosition(TYPE_OF_CHARACTER_ID id,position& pos) {
    auto i = container.find(id);
    if (i!=container.end()) {
        pos = (i->second)->getPosition();
        return true;
    } else {
        return false;
    }
}


template <class T>
iterator_range<typename CharacterContainer<T>::position_to_id_type::const_iterator> CharacterContainer<T>::projection_x_axis(const position& pos, int r) const {
    return {{position_to_id.upper_bound(position(pos.x-r-1,0,0)),position_to_id.upper_bound(position(pos.x+r+1,0,0))}};
}


template <class T>
auto CharacterContainer<T>::find(const std::string &text) const -> pointer {
    try {
        auto id = boost::lexical_cast<TYPE_OF_CHARACTER_ID>(text);
        return find(id);
    } catch (boost::bad_lexical_cast &) {
        for (const auto &character : container) {
            if (comparestrings_nocase(character.second->getName(), text)) {
                return character.second;
            }
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
    const auto i = position_to_id.find(pos);
    
    if (i!=position_to_id.end()) {
        return find(i->second);
    }
    return nullptr;
}


template <class T>
void CharacterContainer<T>::update(pointer p, const position& newPosition) {
    const auto id = p->getId();

    if (!find(id)) {
        return;
    }

    const auto &oldPosition = p->getPosition();
    const auto range = position_to_id.equal_range(oldPosition);
    
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == id) {
            position_to_id.erase(it);
            position_to_id.insert(std::make_pair(newPosition, id));
            return;
        }
    }
}


template <class T>
bool CharacterContainer<T>::erase(TYPE_OF_CHARACTER_ID id) {
    position pos;
    
    if (getPosition(id, pos)) {
        const auto range = position_to_id.equal_range(pos);

        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == id) {
                position_to_id.erase(it);
                break;
            }
        }
    }
    
    return container.erase(id) > 0;
}


template <class T>
auto CharacterContainer<T>::findAllCharactersInRangeOf(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;
    auto candidates = projection_x_axis(pos,distancemetric);
    
    for (auto& c : candidates) {
        const position& p = c.first;
        TYPE_OF_CHARACTER_ID id = c.second;
        short int dx = p.x - pos.x;
        short int dy = p.y - pos.y;
        short int dz = p.z - pos.z;
        
        if (abs(dx) <= distancemetric && abs(dy) <= distancemetric && -RANGEDOWN <= dz && dz <= RANGEUP) {
            if (auto character=find(id)) temp.push_back(character);
        }
    }
    
    return temp;
}


template <class T>
auto CharacterContainer<T>::findAllCharactersInScreen(const position &pos) const -> std::vector<pointer> {
    std::vector<pointer> temp;
    const int MAX_SCREEN_RANGE = 30;
    auto candidates = projection_x_axis(pos,MAX_SCREEN_RANGE);
    
    for (auto& c : candidates) {
        const position& p = c.first;
        TYPE_OF_CHARACTER_ID id = c.second;
        short int dx = p.x - pos.x;
        short int dy = p.y - pos.y;
        short int dz = p.z - pos.z;
    
        if (auto character=find(id)) {
            if ((abs(dx) + abs(dy) <= character->getScreenRange()) && (-RANGEDOWN <= dz) && (dz <= RANGEUP)) {
                temp.push_back(character);
            }
        }
    }

    return temp;
}


template <class T>
auto CharacterContainer<T>::findAllAliveCharactersInRangeOf(const position &pos, int distancemetric) const -> std::vector<pointer> {
    std::vector<pointer> temp;
    auto candidates = projection_x_axis(pos,distancemetric);
    
    for (auto& c : candidates) {
        const position& p = c.first;
        TYPE_OF_CHARACTER_ID id = c.second;
        short int dx = p.x - pos.x;
        short int dy = p.y - pos.y;
        short int dz = p.z - pos.z;
        
        if (abs(dx) <= distancemetric && abs(dy) <= distancemetric && -RANGEDOWN <= dz && dz <= RANGEUP) {
            if (auto character=find(id))
                if (character->isAlive())
                    temp.push_back(character);
        }
    }

    return temp;
}


template <class T>
bool CharacterContainer<T>::findAllCharactersWithXInRangeOf(short int startx, short int endx, std::vector<pointer> &ret) const {
    bool found_one = false;
    int r = (endx-startx)/2+1;
    int x = startx + (endx-startx)/2;
    auto candidates = projection_x_axis(position(x,0,0),r);
    
    for (auto& c : candidates) {
        const position& p = c.first;
        TYPE_OF_CHARACTER_ID id = c.second;
    
        if ((p.x >= startx) && (p.x <= endx)) {
            if (auto character=find(id))
                ret.push_back(character);
        }
    }
    
    return found_one;
}


template class CharacterContainer<Character>;
template class CharacterContainer<Player>;
template class CharacterContainer<Monster>;
template class CharacterContainer<NPC>;
