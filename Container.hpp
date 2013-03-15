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


#ifndef CONTAINER_HPP
#define CONTAINER_HPP

// #define Container_DEBUG

#define MAXIMALEREKURSIONSTIEFE 100

#include "TableStructs.hpp"

#include <unordered_map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sys/types.h>

class RekursionException: public std::exception {};

#include "Item.hpp"
#include "constants.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

class CommonObjectTable;


class Container {
public:
    typedef std::unordered_map<TYPE_OF_CONTAINERSLOTS, Item> ITEMMAP;
    typedef std::unordered_map<TYPE_OF_CONTAINERSLOTS, Container *> CONTAINERMAP;

private:
    Item::id_type itemId;

    ITEMMAP items;
    CONTAINERMAP containers;

public:
    Container(Item::id_type itemId);
    Container(const Container &source);
    ~Container();
    Container &operator =(const Container &source);

    bool TakeItemNr(TYPE_OF_CONTAINERSLOTS nr, Item &it, Container *&cc, Item::number_type count);
    bool viewItemNr(TYPE_OF_CONTAINERSLOTS nr, ScriptItem &it, Container *&cc);
    bool changeQualityAt(TYPE_OF_CONTAINERSLOTS nr, short int amount);
    bool changeQuality(Item::id_type id, short int amount);
    bool InsertContainer(Item it, Container *cc);
    bool InsertContainer(Item it, Container *cc, TYPE_OF_CONTAINERSLOTS pos);
    Item::number_type mergeItem(Item item);
    bool InsertItem(Item it, bool merge);
    bool InsertItem(Item it, TYPE_OF_CONTAINERSLOTS);
    bool InsertItem(Item it);

    void Save(std::ofstream &where);
    void Load(std::istream &where);

    void doAge(bool inventory = false);

    int countItem(Item::id_type itemid, script_data_exchangemap const *data = nullptr) const;

    void increaseItemList(Item::id_type itemid, luabind::object &list,int &index);
    void increaseItemList(luabind::object &list,int &index);

    luabind::object getItemList(Item::id_type itemid);
    luabind::object getItemList();

    int eraseItem(Item::id_type itemid, Item::number_type count, script_data_exchangemap const *data = nullptr);

    int increaseAtPos(unsigned char pos, int count);

    bool swapAtPos(unsigned char pos, Item::id_type newid, Item::quality_type newQuality = 0);

    bool changeItem(ScriptItem &it);

    int weight();

    TYPE_OF_CONTAINERSLOTS getSlotCount();

    inline const ITEMMAP &getItems() const {
        return items;
    }
    inline const CONTAINERMAP &getContainers() const {
        return containers;
    }

private:
    bool isItemStackable(Item item);
    void insertIntoFirstFreeSlot(Item &item);
    void insertIntoFirstFreeSlot(Item &item, Container *container);
    int recursiveWeight(int rekt);
};

#endif

