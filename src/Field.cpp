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


#include "Field.hpp"

#include "data/Data.hpp"
#include "globals.hpp"

//#define Field_DEBUG

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
                clientflags = clientflags | (temp.Modificator & (FLAG_GROUNDLEVEL));
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
            ITEMVECTOR::iterator iterat = items.begin();
            items.insert(iterat, it);
        }

        if (Data::TilesModItems.exists(it.getId())) {
            const auto &temp = Data::TilesModItems[it.getId()];
            clientflags   = clientflags | (temp.Modificator & (FLAG_GROUNDLEVEL));
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
            clientflags   = clientflags | (temp.Modificator & (FLAG_GROUNDLEVEL));
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
#ifdef Field_DEBUG
    std::cout << "increaseTopItem " << count << "\n";
#endif

    if (TakeTopItem(it)) {
        temp = count + it.getNumber();
#ifdef Field_DEBUG
        std::cout << "temp " << temp << "\n";
#endif
#ifdef Field_DEBUG
        std::cout << "it.number:"<<it.getNumber()<<std::endl;
#endif
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


void Field::Save(std::ostream &mapt, std::ostream &obj, std::ostream &warp) {

    mapt.write((char *) & tile, sizeof(tile));
    mapt.write((char *) & music, sizeof(music));
    mapt.write((char *) & clientflags, sizeof(clientflags));
    mapt.write((char *) & extraflags, sizeof(extraflags));

    unsigned char size = items.size();
    obj.write((char *) & size, sizeof(size));

    for (auto it = items.begin(); it < items.end(); ++it) {
        it->save(obj);
    }

    if (IsWarpField()) {
        char b = 1;
        warp.write((char *) & b, sizeof(b));
        warp.write((char *) & warptarget.x, sizeof(warptarget.x));
        warp.write((char *) & warptarget.y, sizeof(warptarget.y));
        warp.write((char *) & warptarget.z, sizeof(warptarget.z));
    } else {
        char b = 0;
        warp.write((char *) & b, sizeof(b));
    }
}


void Field::giveNonPassableItems(ITEMVECTOR &nonpassitems) const {
    for (const auto &item : items) {
        if (Data::TilesModItems.nonPassable(item.getId())) {
            nonpassitems.push_back(item);
        }
    }
}


void Field::giveExportItems(ITEMVECTOR &nonmoveitems) const {
    for (const auto &item : items) {
        if (item.isPermanent()) {
            nonmoveitems.push_back(item);
        } else {
            const auto &common = Data::CommonItems[item.getId()];

            if (common.isValid() && common.AfterInfiniteRot > 0) {
                Item rottenItem = item;
                rottenItem.setId(common.AfterInfiniteRot);
                rottenItem.makePermanent();
                nonmoveitems.push_back(rottenItem);
            }
        }
    }
}


void Field::Load(std::istream &mapt, std::istream &obj, std::istream &warp) {

    mapt.read((char *) & tile, sizeof(tile));
    mapt.read((char *) & music, sizeof(music));
    mapt.read((char *) & clientflags, sizeof(clientflags));
    mapt.read((char *) & extraflags, sizeof(extraflags));

    unsigned char ftemp = 255 - FLAG_NPCONFIELD - FLAG_MONSTERONFIELD - FLAG_PLAYERONFIELD;

    clientflags = clientflags & ftemp;

    MAXCOUNTTYPE size;
    obj.read((char *) & size, sizeof(size));

    items.clear();
    Item temp;

    for (int i = 0; i < size; ++i) {
        temp.load(obj);
        items.push_back(temp);
    }

    char iswarp = 0;
    warp.read((char *) & iswarp, sizeof(iswarp));

    if (iswarp == 1) {
        short int x, y, z;
        warp.read((char *) & x, sizeof(warptarget.x));
        warp.read((char *) & y, sizeof(warptarget.y));
        warp.read((char *) & z, sizeof(warptarget.z));
        SetWarpField(position(x, y, z));
    }

}

int8_t Field::DoAgeItems() {

    int8_t ret = 0;

    if (!items.empty()) {
        auto it = items.begin();

        while (it < items.end()) {
            Item &item = *it;

            if (!item.survivesAgeing()) {
                const auto &tempCommon = Data::CommonItems[item.getId()];

                if (tempCommon.isValid() && item.getId() != tempCommon.ObjectAfterRot) {
#ifdef Field_DEBUG
                    std::cout << "FIELD:Ein Item wird umgewandelt von: " << item.getId() << "  nach: " << tempCommon.ObjectAfterRot << "!\n";
#endif

                    //only set ret to 1 if it wasn't -1 because -1 has the highest priority (forces update of the field and rots container)
                    if (item.getId() != tempCommon.ObjectAfterRot && ret != -1) {
                        ret = 1;
                    }

                    item.setId(tempCommon.ObjectAfterRot);

                    const auto &afterRotCommon = Data::CommonItems[tempCommon.ObjectAfterRot];

                    if (afterRotCommon.isValid()) {
                        item.setWear(afterRotCommon.AgeingSpeed);
                    }

                    ++it;
                } else {
#ifdef Field_DEBUG
                    std::cout << "FIELD:Ein Item wird gel�cht,ID:" << item.getId() << "!\n";
#endif

                    if (item.isContainer()) {
                        erasedcontainers->push_back(item.getNumber());
                        ret = -1;
                    }

                    it = items.erase(it);

                    if (ret != -1) {
                        ret = 1;
                    }

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

    // alle durch Items und Tiles modifizierte Flags l�chen
    clientflags = clientflags & (255 - (FLAG_GROUNDLEVEL));
    extraflags = extraflags & (255 - (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));

    if (Data::Tiles.exists(tile)) {
        const TilesStruct &tt = Data::Tiles[tile];
        clientflags = clientflags | (tt.flags & (FLAG_GROUNDLEVEL));
        extraflags = extraflags | (tt.flags & (FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_SPECIALTILE + FLAG_MAKEPASSABLE));
    }

    for (auto it = items.begin(); it < items.end(); ++it) {
        if (Data::TilesModItems.exists(it->getId())) {
            const auto &tmod = Data::TilesModItems[it->getId()];
            clientflags = clientflags | (tmod.Modificator & (FLAG_GROUNDLEVEL));
            extraflags = extraflags | (tmod.Modificator & (FLAG_SPECIALITEM + FLAG_PENETRATEABLE + FLAG_TRANSPARENT + FLAG_PASSABLE + FLAG_MAKEPASSABLE));
        }
    }
}


void Field::DeleteAllItems() {
    items.clear();
    updateFlags();
}


unsigned char Field::GroundLevel() const {
    return (clientflags & FLAG_GROUNDLEVEL);
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



void Field::SetLevel(unsigned char z) {
    z <<= 4;     // bits an die richtige Position bringen
    z &= FLAG_MAPLEVEL;     // andere bits l�chen
    clientflags &= (255 - FLAG_MAPLEVEL);       // maplevel - bits l�chen
    clientflags |= z;     // maplevel - bits setzen
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

