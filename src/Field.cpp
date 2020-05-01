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


#include "Field.hpp"

#include "data/Data.hpp"
#include "globals.hpp"
#include <limits>


void Field::setTileId(uint16_t id) {
    tile = id;
    updateFlags();
}

uint16_t Field::getTileCode() const {
    return tile;
}

uint16_t Field::getTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return tile & 0x001F;
    } else {
        return tile;
    }
}

uint16_t Field::getSecondaryTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return (tile & 0x03E0) >> 5;
    } else {
        return tile;
    }
}

void Field::setMusicId(uint16_t id) {
    music = id;
}

uint16_t Field::getMusicId() const {
    return music;
}

bool Field::isTransparent() const {
    return getTileId() == TRANSPARENT;
}

TYPE_OF_WALKINGCOST Field::getMovementCost() const {
    if (isWalkable()) {
        auto tileId = getTileId();
        const auto &primaryTile = Data::Tiles[tileId];
        TYPE_OF_WALKINGCOST tileWalkingCost = primaryTile.walkingCost;

        tileId = getSecondaryTileId();
        const auto &secondaryTile = Data::Tiles[tileId];
        uint16_t secondaryWalkingCost = secondaryTile.walkingCost;

        if (secondaryWalkingCost < tileWalkingCost) {
            tileWalkingCost = secondaryWalkingCost;
        }

        return tileWalkingCost;
    } else {
        return std::numeric_limits<TYPE_OF_WALKINGCOST>::max();
    }
}

ScriptItem Field::getStackItem(uint8_t pos) const {
    if (pos < items.size()) {
        ScriptItem result(items.at(pos));
        result.type = ScriptItem::it_field;
        result.itempos = pos;
        return result;
    }

    return {};
}

const std::vector<Item> &Field::getItemStack() const {
    return items;
}

bool Field::addItemOnStack(const Item &item) {
    if (items.size() < MAXITEMS) {
        items.push_back(item);
        updateFlags();

        return true;
    }

    return false;
}


bool Field::addItemOnStackIfWalkable(const Item &item) {
    if (isWalkable()) {
        return addItemOnStack(item);
    }

    return false;
}


bool Field::takeItemFromStack(Item &item) {
    if (items.empty()) {
        return false;
    }

    item = items.back();
    items.pop_back();
    updateFlags();

    return true;
}


int Field::increaseItemOnStack(int count, bool &erased) {
    if (items.empty()) {
        return false;
    }
    
    Item &item = items.back();
    count += item.getNumber();
    auto maxStack = item.getMaxStack();

    if (count > maxStack) {
        item.setNumber(maxStack);
        count -= maxStack;
        erased = false;
    } else if (count <= 0) {
        items.pop_back();
        updateFlags();
        erased = true;
    } else {
        item.setNumber(count);
        count = 0;
        erased = false;
    }

    return count;
}


bool Field::swapItemOnStack(TYPE_OF_ITEM_ID newId, uint16_t newQuality) {
    if (items.empty()) {
        return false;
    }

    Item &item = items.back();
    item.setId(newId);

    if (newQuality > 0) {
        item.setQuality(newQuality);
    }

    const auto &itemStruct = Data::Items[newId];

    if (itemStruct.isValid()) {
        item.setWear(itemStruct.AgeingSpeed);
    }

    updateFlags();
    return true;
}


bool Field::viewItemOnStack(Item &item) const {
    if (items.empty()) {
        return false;
    }

    item = items.back();

    return true;
}


MAXCOUNTTYPE Field::itemCount() const {
    return items.size();
}


bool Field::addContainerOnStackIfWalkable(Item item, Container *container) {
    if (isWalkable()) {
        if (items.size() < MAXITEMS - 1) {
            if (item.isContainer()) {
                MAXCOUNTTYPE count = 0;

                auto iterat = containers.find(count);

                while ((iterat != containers.end()) && (count < (MAXITEMS - 2))) {
                    ++count; 
                    iterat = containers.find(count);
                }

                if (count < MAXITEMS - 1) {
                    if (!container) {
                        container = new Container(item.getId());
                    }

                    containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
                } else {
                    return false;
                }

                item.setNumber(count);

                if (!addItemOnStackIfWalkable(item)) {
                    containers.erase(count);
                } else {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Field::addContainerOnStack(Item item, Container *container) {
    if (item.isContainer()) {
        MAXCOUNTTYPE count = 0;

        auto iterat = containers.find(count);

        while ((iterat != containers.end()) && (count < MAXITEMS - 2)) {
            ++count;
            iterat = containers.find(count);
        }

        if (count < MAXITEMS - 1) {
            if (!container) {
                container = new Container(item.getId());
            }

            containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
        } else {
            return false;
        }

        item.setNumber(count);

        if (!addItemOnStack(item)) {
            containers.erase(count);
        } else {
            return true;
        }
    }

    return false;
}

void Field::save(std::ofstream &mapStream, std::ofstream &itemStream,
                 std::ofstream &warpStream,
                 std::ofstream &containerStream) const {

    mapStream.write((char *) & tile, sizeof(tile));
    mapStream.write((char *) & music, sizeof(music));
    mapStream.write((char *) & flags, sizeof(flags));

    uint8_t itemsSize = items.size();
    itemStream.write((char *) & itemsSize, sizeof(itemsSize));

    for (const auto &item : items) {
        item.save(itemStream);
    }

    if (isWarp()) {
        char b = 1;
        warpStream.write((char *) & b, sizeof(b));
        warpStream.write((char *) & warptarget, sizeof(warptarget));
    } else {
        char b = 0;
        warpStream.write((char *) & b, sizeof(b));
    }

    uint8_t containersSize = containers.size();
    containerStream.write((char *) & containersSize, sizeof(containersSize));
    
    for (const auto &container : containers) {
        containerStream.write((char *) & container.first, sizeof(container.first));
        container.second->Save(containerStream);
    }
}


std::vector<Item> Field::getExportItems() const {
    std::vector<Item> result;

    for (const auto &item : items) {
        if (item.isPermanent()) {
            result.push_back(item);
        } else {
            const auto &itemStruct = Data::Items[item.getId()];

            if (itemStruct.isValid() && itemStruct.AfterInfiniteRot > 0) {
                Item rottenItem = item;
                rottenItem.setId(itemStruct.AfterInfiniteRot);
                rottenItem.makePermanent();
                result.push_back(rottenItem);
            }
        }
    }

    return result;
}

void Field::load(std::ifstream &mapStream, std::ifstream &itemStream,
                 std::ifstream &warpStream, std::ifstream &containerStream) {

    mapStream.read((char *) & tile, sizeof(tile));
    mapStream.read((char *) & music, sizeof(music));
    mapStream.read((char *) & flags, sizeof(flags));

    unsetBits(FLAG_NPCONFIELD | FLAG_MONSTERONFIELD | FLAG_PLAYERONFIELD);

    MAXCOUNTTYPE size;
    itemStream.read((char *) & size, sizeof(size));

    items.clear();

    for (int i = 0; i < size; ++i) {
        Item item;
        item.load(itemStream);
        items.push_back(item);
    }

    char isWarp = 0;
    warpStream.read((char *) & isWarp, sizeof(isWarp));

    if (isWarp == 1) {
        position target;
        warpStream.read((char *) & target, sizeof(warptarget));
        setWarp(target);
    }

    containerStream.read((char *) & size, sizeof(size));

    for (auto &container : containers) {
        delete container.second;
        container.second = nullptr;
    }

    containers.clear();

    for (int i = 0; i < size; ++i) {
        MAXCOUNTTYPE key;
        containerStream.read((char *) & key, sizeof(key));
        
        for (const auto &item : items) {

            if (item.isContainer() && item.getNumber() == key) {
                auto container = new Container(item.getId());
                container->Load(containerStream);
                containers.insert(decltype(containers)::value_type(key, container));
            }
        }
    }

    updateFlags();
}

int8_t Field::age() {
    for (const auto &container : containers) {
        if (container.second) {
            container.second->doAge();
        }
    }

    int8_t ret = 0;

    if (!items.empty()) {
        auto it = items.begin();

        while (it < items.end()) {
            Item &item = *it;

            if (!item.survivesAgeing()) {
                const auto &itemStruct = Data::Items[item.getId()];
                ret = 1;

                if (itemStruct.isValid() && item.getId() != itemStruct.ObjectAfterRot) {
                    item.setId(itemStruct.ObjectAfterRot);

                    const auto &afterRotItemStruct = Data::Items[itemStruct.ObjectAfterRot];

                    if (afterRotItemStruct.isValid()) {
                        item.setWear(afterRotItemStruct.AgeingSpeed);
                    }

                    ++it;
                } else {
                    if (item.isContainer()) {
                        auto iterat = containers.find(item.getNumber());

                        if (iterat != containers.end()) {
                            delete iterat->second;
                            containers.erase(iterat);
                        }
                    }

                    it = items.erase(it);
                }
            } else {
                ++it;
            }
        }
    }

    updateFlags();

    return ret;

}

void Field::updateFlags() {

    unsetBits(FLAG_SPECIALITEM | FLAG_BLOCKPATH | FLAG_MAKEPASSABLE);

    if (Data::Tiles.exists(tile)) {
        const TilesStruct &tt = Data::Tiles[tile];
        setBits(tt.flags & FLAG_BLOCKPATH);
    }

    for (const auto &item : items) {
        if (Data::TilesModItems.exists(item.getId())) {
            const auto &mod = Data::TilesModItems[item.getId()];
            setBits(mod.Modificator & FLAG_SPECIALITEM);

            if (mod.Modificator & FLAG_MAKEPASSABLE) {
                unsetBits(FLAG_BLOCKPATH);
                setBits(FLAG_MAKEPASSABLE);
            } else if (mod.Modificator & FLAG_BLOCKPATH) {
                unsetBits(FLAG_MAKEPASSABLE);
                setBits(FLAG_BLOCKPATH);
            }
        }
    }
}

bool Field::hasMonster() const {
    return anyBitSet(FLAG_MONSTERONFIELD);
}

void Field::setMonster() {
    setBits(FLAG_MONSTERONFIELD);
}

void Field::removeMonster() {
    unsetBits(FLAG_MONSTERONFIELD);
}

bool Field::hasNPC() const {
    return anyBitSet(FLAG_NPCONFIELD);
}

void Field::setNPC() {
    setBits(FLAG_NPCONFIELD);
}

void Field::removeNPC() {
    unsetBits(FLAG_NPCONFIELD);
}

bool Field::hasPlayer() const {
    return anyBitSet(FLAG_PLAYERONFIELD);
}

void Field::setPlayer() {
    setBits(FLAG_PLAYERONFIELD);
}

void Field::removePlayer() {
    unsetBits(FLAG_PLAYERONFIELD);
}

bool Field::isWarp() const {
    return anyBitSet(FLAG_WARPFIELD);
}


void Field::setWarp(const position &pos) {
    warptarget = pos;
    setBits(FLAG_WARPFIELD);
}


void Field::removeWarp() {
    unsetBits(FLAG_WARPFIELD);
}


void Field::getWarp(position &pos) const {
    pos = warptarget;
}


bool Field::hasSpecialItem() const {
    return anyBitSet(FLAG_SPECIALITEM);
}


bool Field::isWalkable() const {
    return !anyBitSet(FLAG_BLOCKPATH) || anyBitSet(FLAG_MAKEPASSABLE);
}


bool Field::moveToPossible() const {
    return isWalkable() && !anyBitSet(FLAG_MONSTERONFIELD | FLAG_NPCONFIELD | FLAG_PLAYERONFIELD);
}

void Field::setChar() {
    setBits(FLAG_PLAYERONFIELD);
}

void Field::removeChar() {
    unsetBits(FLAG_PLAYERONFIELD);
}

inline void Field::setBits(uint8_t bits) {
    flags |= bits;
}

inline void Field::unsetBits(uint8_t bits) {
    flags &= ~bits;
}

inline bool Field::anyBitSet(uint8_t bits) const {
    return (flags & bits) != 0;
}

