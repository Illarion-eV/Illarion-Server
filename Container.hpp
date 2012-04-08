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

#include <vector>
#include <map>
#include <iterator>
#include <iostream>
#include <fstream>
#include <sys/types.h>

class RekursionException: public std::exception {};

#include "Item.hpp"
#include "constants.hpp"
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

class ContainerObjectTable;
class CommonObjectTable;

extern ContainerObjectTable *ContainerItems;
extern CommonObjectTable *CommonItems;
extern CommonStruct tempCommon;

class Container {
private:
    TYPE_OF_ITEM_ID itemId;

public:
    ITEMVECTOR items;

    typedef std::map < MAXCOUNTTYPE , Container *, std::less < MAXCOUNTTYPE > > CONTAINERMAP;
    CONTAINERMAP containers;

    Container(TYPE_OF_ITEM_ID itemId);
    Container(const Container &source);
    ~Container();
    Container &operator =(const Container &source);

    bool TakeItemNr(MAXCOUNTTYPE nr, Item &it, Container* &cc, unsigned char count);
    bool viewItemNr(MAXCOUNTTYPE nr, ScriptItem &it, Container* &cc);
    bool changeQualityAt(MAXCOUNTTYPE nr, short int amount);
    bool changeQuality(TYPE_OF_ITEM_ID id, short int amount);
    bool InsertContainer(Item it, Container *cc);
    bool InsertItem(Item it, bool merge);
    bool InsertItem(Item it, unsigned char pos);
    bool InsertItemOnLoad(Item it);
    bool InsertItem(Item it);

    void Save(std::ofstream *where);
    void Load(std::istream *where);

    void doAge(ITEM_FUNCT funct, bool inventory = false);

    int countItem(TYPE_OF_ITEM_ID itemid);
    int countItem(TYPE_OF_ITEM_ID itemid, uint32_t data);

    void increaseItemList(TYPE_OF_ITEM_ID itemid, luabind::object &list,int &index);
    void increaseItemList(luabind::object &list,int &index);

    luabind::object getItemList(TYPE_OF_ITEM_ID itemid);
    luabind::object getItemList();

    int eraseItem(TYPE_OF_ITEM_ID itemid, int count);
    int eraseItem(TYPE_OF_ITEM_ID itemid, int count, uint32_t data);

    int increaseAtPos(unsigned char pos, int count);

    bool swapAtPos(unsigned char pos, TYPE_OF_ITEM_ID newid, uint16_t newQuality = 0);

    bool changeItem(ScriptItem It);

    int weight(int rekt);

    int _eraseItem(TYPE_OF_ITEM_ID itemid, int count, uint32_t data, bool useData);

    TYPE_OF_CONTAINERSLOTS getSlotCount();

private:

    bool isItemStackable(Item item);
};

#endif

