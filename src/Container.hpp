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


#ifndef _CONTAINER_HPP_
#define _CONTAINER_HPP_

#define MAXIMALEREKURSIONSTIEFE 100

#include "TableStructs.hpp"

#include <map>
#include <iostream>
#include <fstream>

#include "Item.hpp"

class ItemTable;

class RecursionException : public std::exception {};

class Container {
public:
    using ITEMMAP = std::map<TYPE_OF_CONTAINERSLOTS, Item>;
    using CONTAINERMAP = std::map<TYPE_OF_CONTAINERSLOTS, Container *>;

private:
    Item::id_type itemId;

    ITEMMAP items;
    CONTAINERMAP containers;

public:
    explicit Container(Item::id_type itemId);
    Container(const Container &source);
    virtual ~Container();
    auto operator =(const Container &source) -> Container &;

    auto TakeItemNr(TYPE_OF_CONTAINERSLOTS nr, Item &item, Container *&cc, Item::number_type count) -> bool;
    auto viewItemNr(TYPE_OF_CONTAINERSLOTS nr, ScriptItem &item, Container *&cc) -> bool;
    auto changeQualityAt(TYPE_OF_CONTAINERSLOTS nr, short int amount) -> bool;
    auto InsertContainer(const Item &item, Container *cc) -> bool;
    auto InsertContainer(const Item &item, Container *cc, TYPE_OF_CONTAINERSLOTS pos) -> bool;
    auto mergeItem(Item item) -> Item::number_type;
    auto InsertItem(Item item, bool merge) -> bool;
    auto InsertItem(Item item, TYPE_OF_CONTAINERSLOTS) -> bool;
    auto InsertItem(const Item &item) -> bool;

    void Save(std::ofstream &where);
    void Load(std::istream &where);

    void doAge(bool inventory = false);
    void resetWear();

    virtual auto countItem(Item::id_type itemid, script_data_exchangemap const *data = nullptr) const -> int;

    // TODO merge implementations of both addContentToList variations
    void addContentToList(Item::id_type itemid, std::vector<ScriptItem> &list);
    void addContentToList(std::vector<ScriptItem> &list);

    auto getItemList(Item::id_type itemid) -> std::vector<ScriptItem>;
    auto getItemList() -> std::vector<ScriptItem>;

    virtual auto eraseItem(Item::id_type itemid, Item::number_type count, script_data_exchangemap const *data = nullptr) -> int;

    auto increaseAtPos(unsigned char pos, int count) -> int;

    auto swapAtPos(unsigned char pos, Item::id_type newid, Item::quality_type newQuality = 0) -> bool;

    auto changeItem(ScriptItem &item) -> bool;

    auto weight() -> int;

    [[nodiscard]] virtual auto getSlotCount() const -> TYPE_OF_CONTAINERSLOTS;

    [[nodiscard]] inline auto getItems() const -> const ITEMMAP & {
        return items;
    }
    [[nodiscard]] inline auto getContainers() const -> const CONTAINERMAP & {
        return containers;
    }

    [[nodiscard]] auto getFirstFreeSlot() const -> TYPE_OF_CONTAINERSLOTS;

    [[nodiscard]] inline auto isDepot() const -> bool {
        return itemId == DEPOTITEM;
    }

private:
    auto isItemStackable(const Item &item) -> bool;
    void insertIntoFirstFreeSlot(Item &item);
    void insertIntoFirstFreeSlot(Item &item, Container *container);
    auto recursiveWeight(int rekt) -> int;
};

#endif

