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

Field::Field() : warptarget{0, 0, 0} {
    tile = 0;
    music = 0;
    clientflags = 0;
    extraflags = 0;
}

void Field::setTileId(unsigned short int id) {
    tile = id;
}

unsigned short int Field::getTileCode() const {
    return tile;
}

unsigned short int Field::getTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return tile & 0x001F;
    } else {
        return tile;
    }
}

unsigned short int Field::getSecondaryTileId() const {
    if (((tile & 0xFC00) >> 10) > 0) {
        return (tile & 0x03E0) >> 5;
    } else {
        return tile;
    }
}

void Field::setMusicId(unsigned short int id) {
    music = id;
}

unsigned short int Field::getMusicId() const {
    return music;
}

bool Field::isTransparent() const {
    return getTileId() == TRANSPARENT;
}

TYPE_OF_WALKINGCOST Field::getMovementCost() const {
    if (IsPassable()) {
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

ScriptItem Field::getStackItem(uint8_t spos) const {
    ScriptItem retItem;

    if (items.empty()) {
        return retItem;
    } else {
        uint8_t counter = 0;

        for (const auto &item : items) {
            if (counter >= spos) {
                retItem = item;
                retItem.type = ScriptItem::it_field;
                retItem.itempos = counter;
                return retItem;
            }

            ++counter;
        }

        return retItem;
    }
}


bool Field::addTopItem(const Item &it) {

    if (IsPassable()) {
        if (items.size() < MAXITEMS) {
            items.push_back(it);

            if (Data::TilesModItems.exists(it.getId())) {
                const auto &temp = Data::TilesModItems[it.getId()];
                extraflags = extraflags | (temp.Modificator & (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));
            }

            return true;
        }
    }

    return false;

}


bool Field::PutGroundItem(const Item &it) {
    if (items.size() < MAXITEMS) {
        if (items.empty()) {
            items.push_back(it);
        } else {
            items.insert(items.begin(), it);
        }

        if (Data::TilesModItems.exists(it.getId())) {
            const auto &temp = Data::TilesModItems[it.getId()];
            extraflags   = extraflags | (temp.Modificator & (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));
        }

        return true;
    }

    return false;
}


bool Field::PutTopItem(const Item &it) {
    if (items.size() < MAXITEMS) {
        items.push_back(it);

        if (Data::TilesModItems.exists(it.getId())) {
            const auto &temp = Data::TilesModItems[it.getId()];
            extraflags   = extraflags | (temp.Modificator & (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));
        }

        return true;
    }

    return false;
}


bool Field::TakeTopItem(Item &it) {
    if (items.empty()) {
        return false;
    }

    it = items.back();
    items.pop_back();
    updateFlags();

    return true;
}

bool Field::changeQualityOfTopItem(short int amount) {
    Item it;

    if (TakeTopItem(it)) {
        short int tmpQuality = ((amount+it.getDurability())<100) ? (amount + it.getQuality()) : (it.getQuality() - it.getDurability() + 99);

        if (tmpQuality%100 > 1) {
            it.setQuality(tmpQuality);
            PutTopItem(it);
            return false;
        } else {
            return true;
        }
    }

    return false;
}


int Field::increaseTopItem(int count, bool &erased) {

    Item it;
    int temp = count;

    if (TakeTopItem(it)) {
        temp = count + it.getNumber();
        auto maxStack = it.getMaxStack();

        if (temp > maxStack) {
            it.setNumber(maxStack);
            temp = temp - maxStack;
            PutTopItem(it);
            erased = false;
        } else if (temp <= 0) {
            temp = count + it.getNumber();
            erased = true;
        } else {
            it.setNumber(temp);
            temp = 0;
            PutTopItem(it);
            erased = false;
        }
    }

    return temp;
}


bool Field::swapTopItem(TYPE_OF_ITEM_ID newid, uint16_t newQuality) {
    Item temp;

    if (TakeTopItem(temp)) {
        temp.setId(newid);

        if (newQuality > 0) {
            temp.setQuality(newQuality);
        }

        const auto &common = Data::CommonItems[newid];

        if (common.isValid()) {
            temp.setWear(common.AgeingSpeed);
        }

        PutTopItem(temp);
        return true;
    }

    return false;
}


bool Field::ViewTopItem(Item &it) const {

    if (items.empty()) {
        return false;
    }

    it = items.back();

    return true;
}


MAXCOUNTTYPE Field::NumberOfItems() const {
    return items.size();
}


bool Field::addContainer(Item item, Container *container) {
    if (IsPassable()) {
        if (items.size() < MAXITEMS - 1) {
            if (item.isContainer()) {
                MAXCOUNTTYPE count = 0;

                auto iterat = containers.find(count);

                while ((iterat != containers.end()) && (count < (MAXITEMS - 2))) {
                    ++count; 
                    iterat = containers.find(count);
                }

                if (count < MAXITEMS - 1) {
                    containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
                } else {
                    return false;
                }

                item.setNumber(count);

                if (!addTopItem(item)) {
                    containers.erase(count);
                } else {
                    return true;
                }
            }
        }
    }

    return false;
}

bool Field::addContainerAlways(Item item, Container *container) {
    if (item.isContainer()) {
        MAXCOUNTTYPE count = 0;

        auto iterat = containers.find(count);

        while ((iterat != containers.end()) && (count < MAXITEMS - 2)) {
            ++count;
            iterat = containers.find(count);
        }

        if (count < MAXITEMS - 1) {
            containers.insert(iterat, Container::CONTAINERMAP::value_type(count, container));
        } else {
            return false;
        }

        item.setNumber(count);

        if (!PutTopItem(item)) {
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
    mapStream.write((char *) & clientflags, sizeof(clientflags));
    mapStream.write((char *) & extraflags, sizeof(extraflags));

    unsigned char itemsSize = items.size();
    itemStream.write((char *) & itemsSize, sizeof(itemsSize));

    for (const auto &item : items) {
        item.save(itemStream);
    }

    if (IsWarpField()) {
        char b = 1;
        warpStream.write((char *) & b, sizeof(b));
        warpStream.write((char *) & warptarget, sizeof(warptarget));
    } else {
        char b = 0;
        warpStream.write((char *) & b, sizeof(b));
    }

    unsigned char containersSize = containers.size();
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
            const auto &common = Data::CommonItems[item.getId()];

            if (common.isValid() && common.AfterInfiniteRot > 0) {
                Item rottenItem = item;
                rottenItem.setId(common.AfterInfiniteRot);
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
    mapStream.read((char *) & clientflags, sizeof(clientflags));
    mapStream.read((char *) & extraflags, sizeof(extraflags));

    unsigned char ftemp = 255 - FLAG_NPCONFIELD - FLAG_MONSTERONFIELD - FLAG_PLAYERONFIELD;
    clientflags = clientflags & ftemp;

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
        SetWarpField(target);
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
                containers.emplace(key, container);
            }
        }
    }

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
                const auto &tempCommon = Data::CommonItems[item.getId()];

                if (tempCommon.isValid() && item.getId() != tempCommon.ObjectAfterRot) {
                    if (item.getId() != tempCommon.ObjectAfterRot) {
                        ret = 1;
                    }

                    item.setId(tempCommon.ObjectAfterRot);

                    const auto &afterRotCommon = Data::CommonItems[tempCommon.ObjectAfterRot];

                    if (afterRotCommon.isValid()) {
                        item.setWear(afterRotCommon.AgeingSpeed);
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

                    ret = 1;
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

    extraflags = extraflags & (255 - (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));

    if (Data::Tiles.exists(tile)) {
        const TilesStruct &tt = Data::Tiles[tile];
        extraflags = extraflags | (tt.flags & (FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_SPECIALTILE + FLAG_MAKEPASSABLE));
    }

    for (auto it = items.begin(); it < items.end(); ++it) {
        if (Data::TilesModItems.exists(it->getId())) {
            const auto &tmod = Data::TilesModItems[it->getId()];
            extraflags = extraflags | (tmod.Modificator & (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));
        }
    }
}


void Field::DeleteAllItems() {
    items.clear();
    updateFlags();
}


bool Field::IsMonsterOnField() const {
    return ((clientflags & FLAG_MONSTERONFIELD) != 0);
}


void Field::SetMonsterOnField(bool t) {
    if (t) {
        clientflags = clientflags | FLAG_MONSTERONFIELD;
    } else {
        clientflags = clientflags & (255 - FLAG_MONSTERONFIELD);
    }
}


bool Field::IsNPCOnField() const {
    return ((clientflags & FLAG_NPCONFIELD) != 0);
}


void Field::SetNPCOnField(bool t) {
    if (t) {
        clientflags = clientflags | FLAG_NPCONFIELD;
    } else {
        clientflags = clientflags & (255 - FLAG_NPCONFIELD);
    }
}


bool Field::IsPlayerOnField() const {
    return ((clientflags & FLAG_PLAYERONFIELD) != 0);
}


void Field::SetPlayerOnField(bool t) {
    if (t) {
        clientflags = clientflags | FLAG_PLAYERONFIELD;
    } else {
        clientflags = clientflags & (255 - FLAG_PLAYERONFIELD);
    }
}


bool Field::IsWarpField() const {
    return ((extraflags & FLAG_WARPFIELD) != 0);
}


void Field::SetWarpField(const position &pos) {
    warptarget = pos;
    extraflags = extraflags | FLAG_WARPFIELD;
}


void Field::UnsetWarpField() {
    extraflags = extraflags & (255 - FLAG_WARPFIELD);
}


void Field::GetWarpField(position &pos) const {
    pos = warptarget;
}


bool Field::HasSpecialItem() const {
    return ((extraflags & FLAG_SPECIALITEM) != 0);
}


void Field::SetSpecialItem(bool t) {
    if (t) {
        extraflags = extraflags | FLAG_SPECIALITEM;
    } else {
        extraflags = extraflags & (255 - FLAG_SPECIALITEM);
    }
}


bool Field::IsSpecialField() const {
    return ((extraflags & FLAG_SPECIALTILE) != 0);
}


void Field::SetSpecialField(bool t) {
    if (t) {
        extraflags = extraflags | FLAG_SPECIALTILE;
    } else {
        extraflags = extraflags & (255 - FLAG_SPECIALTILE);
    }
}


bool Field::IsTransparent() const {
    return ((extraflags & FLAG_TRANSPARENT) == 0);
}


bool Field::IsPassable() const {
    return (((extraflags & FLAG_PASSABLE) == 0) || ((extraflags & FLAG_MAKEPASSABLE) != 0));
}


bool Field::IsPenetrateable() const {
    return ((extraflags & FLAG_PENETRATEABLE) == 0);
}


bool Field::moveToPossible() const {
    return (
               (IsPassable()  &&
                ((clientflags & (FLAG_MONSTERONFIELD | FLAG_NPCONFIELD | FLAG_PLAYERONFIELD)) == 0))
           );
}

void Field::setChar() {
    clientflags |= FLAG_PLAYERONFIELD;
}

void Field::removeChar() {
    clientflags &= ~FLAG_PLAYERONFIELD;
}

