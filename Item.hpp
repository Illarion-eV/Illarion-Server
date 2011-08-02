/*
 * illarionserver - server for the game Illarion
 * Copyright 2011 Illarion e.V.
 *
 * This file is part of illarionserver.
 *
 * illarionserver is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * illarionserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with illarionserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEM_HH
#define ITEM_HH

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "globals.hpp"
#include "fuse_ptr.hpp"

#define TYPE_OF_ITEM_ID uint16_t

class Character;
class Container;

class Item {
public:
    TYPE_OF_ITEM_ID id;
    uint8_t number;
    uint8_t wear;
    uint16_t quality;
    uint32_t data;

    Item(): id(0), number(0), wear(0), quality(333), data(0), data_map(1) {}
    Item(TYPE_OF_ITEM_ID _id, unsigned char _number, unsigned char _wear, uint16_t _quality = 333, uint32_t _data = 0) :
        id(_id), number(_number), wear(_wear), quality(_quality), data(_data),data_map(1) {}

    std::string getData(std::string key);
    void setData(std::string key, std::string value);
    void save(std::ostream *obj);
    void load(std::istream *obj);

    typedef boost::unordered_map<std::string, std::string> DATA_MAP;
    DATA_MAP data_map;
};

class ScriptItem : public Item {
public:
    enum itemtype {
        notdefined = 0,
        it_showcase1 = 1,
        it_showcase2 = 2,
        it_field = 3,
        it_inventory = 4,
        it_belt = 5,
        it_container = 6
    };

    itemtype type;
    position pos;
    unsigned char itempos;
    Character *owner;
    fuse_ptr<Character> getOwnerForLua() {
        fuse_ptr<Character> fuse_owner(owner);
        return fuse_owner;
    };
    Container *inside;
    ScriptItem() : Item(0,0,0), type(notdefined), pos(position(0,0,0)), itempos(255), owner(NULL), inside(NULL) {}
    unsigned char getType() {
        return type;
    }
    ScriptItem(const Item &source) : Item(source) {
        itempos = 0;
        pos = position(0,0,0);
        type = notdefined;
        owner = NULL;
        inside = NULL;
    }

} ;

typedef std::vector < Item > ITEMVECTOR;

typedef bool (*ITEM_FUNCT)(Item *);

#endif
